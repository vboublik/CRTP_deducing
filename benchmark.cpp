#include <iostream>
#include <fstream>
#include <tuple>
#include <variant>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <algorithm>
#include <random>
#include <cstdio>
#include <functional>

// ================================================================
// КОЛИ СТАТИЧНЕ ПРОГРАЄ ДИНАМІЧНОМУ
// Межа проблеми виразності для CRTP-кортежних інфраструктур
// Володимир Бублик, НаУКМА — TAAPSD 2026
// ================================================================
//
// Принципи вимірювання:
//  * однаковий обсяг роботи в усіх варіантах: кожен звіт = header+body+footer,
//    тобто 3 записи у volatile sink на один звіт;
//  * кожне вимірювання повторюється RUNS разів, береться медіана;
//  * Сценарій A — склад відомий під час компіляції (3 звіти);
//    Сценарій B — склад визначається під час виконання (N звітів
//    у випадковому порядку); tuple+fold тут принципово не застосовний.

static volatile long long sink = 0; // запобігає видаленню роботи оптимізатором

// ================================================================
// 1. GoF: Шаблонний метод через virtual dispatch
// ================================================================
namespace gof {
    struct IReport {
        void generate() { header(); body(); footer(); }
        virtual ~IReport() = default;
    private:
        virtual void header() = 0;
        virtual void body()   = 0;
        virtual void footer() = 0;
    };
    struct PdfReport : IReport {
    private:
        void header() override { sink += 1; }
        void body()   override { sink += 2; }
        void footer() override { sink += 3; }
    };
    struct HtmlReport : IReport {
    private:
        void header() override { sink += 4; }
        void body()   override { sink += 5; }
        void footer() override { sink += 6; }
    };
    struct CsvReport : IReport {
    private:
        void header() override { sink += 7; }
        void body()   override { sink += 8; }
        void footer() override { sink += 9; }
    };
} // namespace gof

// ================================================================
// 2. CRTP: статичний поліморфізм
// ================================================================
namespace crtp {
    template<typename Derived>
    struct Report {
        void generate() {
            static_cast<Derived*>(this)->header();
            static_cast<Derived*>(this)->body();
            static_cast<Derived*>(this)->footer();
        }
    };
    struct PdfReport : Report<PdfReport> {
        void header() { sink += 1; }
        void body()   { sink += 2; }
        void footer() { sink += 3; }
    };
    struct HtmlReport : Report<HtmlReport> {
        void header() { sink += 4; }
        void body()   { sink += 5; }
        void footer() { sink += 6; }
    };
    struct CsvReport : Report<CsvReport> {
        void header() { sink += 7; }
        void body()   { sink += 8; }
        void footer() { sink += 9; }
    };
} // namespace crtp

// ================================================================
// 3. Deducing this (C++23)
// ================================================================
namespace deduced {
    struct Report {
        void generate(this auto& self) {
            self.header();
            self.body();
            self.footer();
        }
    };
    struct PdfReport : Report {
        void header() { sink += 1; }
        void body()   { sink += 2; }
        void footer() { sink += 3; }
    };
    struct HtmlReport : Report {
        void header() { sink += 4; }
        void body()   { sink += 5; }
        void footer() { sink += 6; }
    };
    struct CsvReport : Report {
        void header() { sink += 7; }
        void body()   { sink += 8; }
        void footer() { sink += 9; }
    };
} // namespace deduced

// ================================================================
// 4–5. Звіти без ієрархії для tuple+fold і variant+visit.
// ТОЙ САМИЙ обсяг роботи, що й у 1–3: header+body+footer.
// ================================================================
namespace plain {
    struct PdfReport {
        void generate() { header(); body(); footer(); }
        void header() { sink += 1; }
        void body()   { sink += 2; }
        void footer() { sink += 3; }
    };
    struct HtmlReport {
        void generate() { header(); body(); footer(); }
        void header() { sink += 4; }
        void body()   { sink += 5; }
        void footer() { sink += 6; }
    };
    struct CsvReport {
        void generate() { header(); body(); footer(); }
        void header() { sink += 7; }
        void body()   { sink += 8; }
        void footer() { sink += 9; }
    };

    template<typename... Reports>
    struct Runner {
        std::tuple<Reports...> reports;
        void run_all() {
            std::apply([](auto&... r) { (r.generate(), ...); }, reports);
        }
    };

    using ReportVariant = std::variant<PdfReport, HtmlReport, CsvReport>;
} // namespace plain


// ================================================================
// 6. Type erasure (Parent, GoingNative 2013): відкритий набір типів
//    без спільного базового класу. Статика на вході (концепт Report),
//    динаміка всередині (прихована ієрархія Concept / Model<T>)
// ================================================================
namespace type_erasure {
    // «Інтерфейс» без базового класу: перевіряється під час компіляції (C++20)
    template <typename T>
    concept Report = requires(T& r) { r.generate(); };

    class AnyReport {
        struct Concept {                                   // прихований інтерфейс
            virtual ~Concept() = default;
            virtual void generate() = 0;
        };
        template <Report T>
        struct Model final : Concept {                     // зв'язує будь-який T з інтерфейсом
            T data;
            explicit Model(T val) : data(std::move(val)) {}
            void generate() override { data.generate(); }
        };
        std::unique_ptr<Concept> self;
    public:
        template <Report T>
        AnyReport(T report) : self(std::make_unique<Model<T>>(std::move(report))) {}
        void generate() { self->generate(); }
    };
} // namespace type_erasure

// ================================================================
// 7. std::function: стирається не об'єкт, а дія (одна операція)
// ================================================================
namespace std_function {
    using Action = std::function<void()>;
    template<typename T>
    Action make_action(T t) { return [r = std::move(t)]() mutable { r.generate(); }; }
} // namespace std_function

// ================================================================
// Вимірювання: RUNS прогонів по reps повторів, медіана
// ================================================================
constexpr int RUNS = 7;

template<class F>
double median_ms(F&& f, int reps) {
    std::vector<double> t;
    f(); // розігрів
    for (int run = 0; run < RUNS; ++run) {
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < reps; ++i) f();
        auto t1 = std::chrono::steady_clock::now();
        t.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(t.begin(), t.end());
    return t[t.size() / 2];
}

int main(int argc, char** argv) {
    // ------------------------------------------------------------
    // Сценарій A: склад відомий під час компіляції (3 звіти)
    // ------------------------------------------------------------
    const int REPS_A = 1'000'000;

    std::vector<std::unique_ptr<gof::IReport>> gof_a;
    gof_a.push_back(std::make_unique<gof::PdfReport>());
    gof_a.push_back(std::make_unique<gof::HtmlReport>());
    gof_a.push_back(std::make_unique<gof::CsvReport>());
    double a_gof = median_ms([&] { for (auto& r : gof_a) r->generate(); }, REPS_A);

    crtp::PdfReport cp; crtp::HtmlReport ch; crtp::CsvReport cc;
    double a_crtp = median_ms([&] { cp.generate(); ch.generate(); cc.generate(); }, REPS_A);

    deduced::PdfReport dp; deduced::HtmlReport dh; deduced::CsvReport dc;
    double a_ded = median_ms([&] { dp.generate(); dh.generate(); dc.generate(); }, REPS_A);

    plain::Runner<plain::PdfReport, plain::HtmlReport, plain::CsvReport> runner;
    double a_tuple = median_ms([&] { runner.run_all(); }, REPS_A);

    std::vector<plain::ReportVariant> var_a = {
        plain::PdfReport{}, plain::HtmlReport{}, plain::CsvReport{} };
    double a_var = median_ms([&] {
        for (auto& r : var_a) std::visit([](auto& rep) { rep.generate(); }, r);
    }, REPS_A);

    std::vector<type_erasure::AnyReport> te_a;
    te_a.emplace_back(plain::PdfReport{}); te_a.emplace_back(plain::HtmlReport{}); te_a.emplace_back(plain::CsvReport{});
    double a_te = median_ms([&] { for (auto& r : te_a) r.generate(); }, REPS_A);

    std::vector<std_function::Action> sf_a{
        std_function::make_action(plain::PdfReport{}),
        std_function::make_action(plain::HtmlReport{}),
        std_function::make_action(plain::CsvReport{}) };
    double a_sf = median_ms([&] { for (auto& f : sf_a) f(); }, REPS_A);

    // ------------------------------------------------------------
    // Сценарій B: склад визначається під час виконання — випадок variant.
    // Випадкові числа імітують вибір користувача (read_user_choice у тезах):
    // компілятор не знає наперед ні типів, ні порядку звітів,
    // тому не може прибрати диспетчеризацію.
    // Зерно (seed) робить цей «вибір» однаковим для всіх, хто запускає тест.
    // Запуск: benchmark [N] [seed]
    // ------------------------------------------------------------
    const int N = (argc > 1) ? std::stoi(argv[1]) : 3000;
    const unsigned seed = (argc > 2) ? static_cast<unsigned>(std::stoul(argv[2]))
                                     : std::random_device{}();
    const int REPS_B = std::max(1, REPS_A * 3 / N); // той самий загальний обсяг викликів

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> pick(0, 2);
    std::vector<int> kinds(N);
    for (auto& k : kinds) k = pick(rng);

    std::vector<std::unique_ptr<gof::IReport>> gof_b;
    std::vector<plain::ReportVariant> var_b;
    std::vector<type_erasure::AnyReport> te_b;
    std::vector<std_function::Action> sf_b;
    gof_b.reserve(N); var_b.reserve(N); te_b.reserve(N); sf_b.reserve(N);
    for (int k : kinds) {
        switch (k) {
        case 0: gof_b.push_back(std::make_unique<gof::PdfReport>());  var_b.emplace_back(plain::PdfReport{});
                te_b.emplace_back(plain::PdfReport{});  sf_b.push_back(std_function::make_action(plain::PdfReport{}));  break;
        case 1: gof_b.push_back(std::make_unique<gof::HtmlReport>()); var_b.emplace_back(plain::HtmlReport{});
                te_b.emplace_back(plain::HtmlReport{}); sf_b.push_back(std_function::make_action(plain::HtmlReport{})); break;
        default: gof_b.push_back(std::make_unique<gof::CsvReport>()); var_b.emplace_back(plain::CsvReport{});
                te_b.emplace_back(plain::CsvReport{});  sf_b.push_back(std_function::make_action(plain::CsvReport{}));  break;
        }
    }
    double b_gof = median_ms([&] { for (auto& r : gof_b) r->generate(); }, REPS_B);
    double b_var = median_ms([&] {
        for (auto& r : var_b) std::visit([](auto& rep) { rep.generate(); }, r);
    }, REPS_B);
    double b_te = median_ms([&] { for (auto& r : te_b) r.generate(); }, REPS_B);
    double b_sf = median_ms([&] { for (auto& f : sf_b) f(); }, REPS_B);

    // ------------------------------------------------------------
    // Сценарій B, окремо: побудова колекції (виділення пам'яті).
    // У вимірювання обходу вище вона НЕ входить: колекції будуються заздалегідь.
    // Тут — час побудови й знищення колекції з N звітів, у мікросекундах.
    // ------------------------------------------------------------
    const int REPS_BUILD = 200;
    auto build_us = [&](auto make_one) {
        return 1000.0 * median_ms([&] {
            auto v = make_one();                       // побудова
            sink += static_cast<long long>(v.size());  // знищення — при виході з лямбди
        }, REPS_BUILD) / REPS_BUILD;
    };
    double c_gof = build_us([&] {
        std::vector<std::unique_ptr<gof::IReport>> v; v.reserve(N);
        for (int k : kinds) {
            if (k == 0) v.push_back(std::make_unique<gof::PdfReport>());
            else if (k == 1) v.push_back(std::make_unique<gof::HtmlReport>());
            else v.push_back(std::make_unique<gof::CsvReport>());
        }
        return v; });
    double c_var = build_us([&] {
        std::vector<plain::ReportVariant> v; v.reserve(N);
        for (int k : kinds) {
            if (k == 0) v.emplace_back(plain::PdfReport{});
            else if (k == 1) v.emplace_back(plain::HtmlReport{});
            else v.emplace_back(plain::CsvReport{});
        }
        return v; });
    double c_te = build_us([&] {
        std::vector<type_erasure::AnyReport> v; v.reserve(N);
        for (int k : kinds) {
            if (k == 0) v.emplace_back(plain::PdfReport{});
            else if (k == 1) v.emplace_back(plain::HtmlReport{});
            else v.emplace_back(plain::CsvReport{});
        }
        return v; });
    double c_sf = build_us([&] {
        std::vector<std_function::Action> v; v.reserve(N);
        for (int k : kinds) {
            if (k == 0) v.push_back(std_function::make_action(plain::PdfReport{}));
            else if (k == 1) v.push_back(std_function::make_action(plain::HtmlReport{}));
            else v.push_back(std_function::make_action(plain::CsvReport{}));
        }
        return v; });

    // ------------------------------------------------------------
    // Результати: і в консоль, і у файл
    // ------------------------------------------------------------
    auto print_results = [&](std::ostream& out) {
        char buf[96];
        auto row = [&](const char* label, double v, const char* unit = "ms") {
            std::snprintf(buf, sizeof(buf), "%-36s %8.2f %s\n", label, v, unit);
            out << buf;
        };
        out << "\n=== TAAPSD 2026: dispatch benchmark ===\n";
        out << "median of " << RUNS << " runs; every report = header+body+footer\n";

        out << "\nScenario A: composition fixed at compile time (3 reports x "
            << REPS_A << " reps)\n";
        row("1. GoF (virtual dispatch)", a_gof);
        row("2. CRTP", a_crtp);
        row("3. Deducing this (C++23)", a_ded);
        row("4. tuple + fold", a_tuple);
        row("5. variant + visit", a_var);
        row("6. type erasure (AnyReport)", a_te);
        row("7. std::function (action)", a_sf);

        out << "\nScenario B: composition determined at runtime (N=" << N
            << " random reports x " << REPS_B << " reps, seed=" << seed << ")\n";
        row("1. GoF (virtual dispatch)", b_gof);
        out << "2-4. CRTP / deducing this / tuple   not expressible\n";
        row("5. variant + visit", b_var);
        row("6. type erasure (AnyReport)", b_te);
        row("7. std::function (action)", b_sf);

        out << "\nScenario B, building the collection once (N=" << N
            << " reports: allocation + destruction, not included above), microseconds\n";
        row("1. GoF (unique_ptr per object)", c_gof, "us");
        row("5. variant (inline, no allocation)", c_var, "us");
        row("6. type erasure (heap per object)", c_te, "us");
        row("7. std::function (small buffer)", c_sf, "us");

        out << "\n(sink=" << sink << ")\n";
    };

    print_results(std::cout);
    std::ofstream fout("benchmark_results.txt");
    if (fout) {
        print_results(fout);
        std::cout << "=> results saved to benchmark_results.txt\n";
    }
    return 0;
}
