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

// ================================================================
// ПРО МЕЖІ ВИРАЗНОСТІ CRTP-КОРТЕЖНИХ ІНФРАСТРУКТУР В УМОВАХ RUNTIME
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

    // ------------------------------------------------------------
    // Сценарій B: склад визначається під час виконання — випадок variant.
    // Випадкові числа імітують вибір користувача (read_user_choice у тезах):
    // компілятор не знає наперед ні типів, ні порядку звітів,
    // тому не може прибрати диспетчеризацію.
    // Зерно (seed) робить цей «вибір» однаковим для всіх, хто запускає тест.
    // Запуск: main [N] [seed]
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
    gof_b.reserve(N); var_b.reserve(N);
    for (int k : kinds) {
        switch (k) {
        case 0: gof_b.push_back(std::make_unique<gof::PdfReport>());  var_b.emplace_back(plain::PdfReport{});  break;
        case 1: gof_b.push_back(std::make_unique<gof::HtmlReport>()); var_b.emplace_back(plain::HtmlReport{}); break;
        default: gof_b.push_back(std::make_unique<gof::CsvReport>()); var_b.emplace_back(plain::CsvReport{}); break;
        }
    }
    double b_gof = median_ms([&] { for (auto& r : gof_b) r->generate(); }, REPS_B);
    double b_var = median_ms([&] {
        for (auto& r : var_b) std::visit([](auto& rep) { rep.generate(); }, r);
    }, REPS_B);

    // ------------------------------------------------------------
    // Результати: і в консоль, і у файл
    // ------------------------------------------------------------
    auto print_results = [&](std::ostream& out) {
        char buf[96];
        auto row = [&](const char* label, double ms) {
            std::snprintf(buf, sizeof(buf), "%-34s %8.2f ms\n", label, ms);
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

        out << "\nScenario B: composition determined at runtime (N=" << N
            << " random reports x " << REPS_B << " reps, seed=" << seed << ")\n";
        row("1. GoF (virtual dispatch)", b_gof);
        out << "2-4. CRTP / deducing this / tuple   not expressible\n";
        row("5. variant + visit", b_var);

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
