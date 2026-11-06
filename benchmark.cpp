#include <iostream>
#include <fstream>
#include <tuple>
#include <variant>
#include <vector>
#include <string>
#include <chrono>
#include <numeric>

// ================================================================
// КОЛИ СТАТИЧНЕ ПРОГРАЄ ДИНАМІЧНОМУ
// Межа проблеми виразності для CRTP-кортежних інфраструктур
// Володимир Бублик, НаУКМА — TAAPSD 2026
// ================================================================

// ----------------------------------------------------------------
// Спільні типи звітів (без поведінки — щоб не вимірювати cout)
// ----------------------------------------------------------------

static volatile long long sink = 0; // запобігає оптимізації

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
// 4. tuple + fold: compile-time фіксований набір
// ================================================================

namespace tuple_fold {

    struct PdfReport  { void generate() { sink += 1; } };
    struct HtmlReport { void generate() { sink += 4; } };
    struct CsvReport  { void generate() { sink += 7; } };

    template<typename... Reports>
    struct Runner {
        std::tuple<Reports...> reports;
        void run_all() {
            std::apply([](auto&... r) { (r.generate(), ...); }, reports);
        }
    };

} // namespace tuple_fold

// ================================================================
// 5. variant + visit: runtime-визначений набір
// ================================================================

namespace variant_visit {

    struct PdfReport  { void generate() { sink += 1; } };
    struct HtmlReport { void generate() { sink += 4; } };
    struct CsvReport  { void generate() { sink += 7; } };

    using ReportVariant = std::variant<PdfReport, HtmlReport, CsvReport>;

} // namespace variant_visit

// ================================================================
// Бенчмарк
// ================================================================

template<class F>
double time_ms(F&& f, int reps) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < reps; ++i) f();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    const int REPS = 1'000'000;

    // --- 1. GoF ---
    std::vector<gof::IReport*> gof_reports = {
        new gof::PdfReport{},
        new gof::HtmlReport{},
        new gof::CsvReport{}
    };
    double t_gof = time_ms([&] {
        for (auto* r : gof_reports) r->generate();
    }, REPS);
    for (auto* r : gof_reports) delete r;

    // --- 2. CRTP ---
    crtp::PdfReport  crtp_pdf;
    crtp::HtmlReport crtp_html;
    crtp::CsvReport  crtp_csv;
    double t_crtp = time_ms([&] {
        crtp_pdf.generate();
        crtp_html.generate();
        crtp_csv.generate();
    }, REPS);

    // --- 3. Deducing this ---
    deduced::PdfReport  ded_pdf;
    deduced::HtmlReport ded_html;
    deduced::CsvReport  ded_csv;
    double t_deduced = time_ms([&] {
        ded_pdf.generate();
        ded_html.generate();
        ded_csv.generate();
    }, REPS);

    // --- 4. tuple + fold ---
    tuple_fold::Runner<
        tuple_fold::PdfReport,
        tuple_fold::HtmlReport,
        tuple_fold::CsvReport> runner;
    double t_tuple = time_ms([&] {
        runner.run_all();
    }, REPS);

    // --- 5. variant + visit ---
    std::vector<variant_visit::ReportVariant> vreports = {
        variant_visit::PdfReport{},
        variant_visit::HtmlReport{},
        variant_visit::CsvReport{}
    };
    double t_variant = time_ms([&] {
        for (auto& r : vreports)
            std::visit([](auto& rep) { rep.generate(); }, r);
    }, REPS);

    // --- Результати ---
    // виводимо і в консоль, і у файл
    auto print_results = [&](std::ostream& out) {
        out << "\n=== TAAPSD 2026: dispatch benchmark ===\n";
        out << "REPS = " << REPS << "  |  sink=" << sink << "\n\n";
        auto row = [&](const char* label, double ms) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%-32s %8.2f ms\n", label, ms);
            out << buf;
        };
        row("1. GoF (virtual dispatch)",  t_gof);
        row("2. CRTP",                    t_crtp);
        row("3. Deducing this (C++23)",   t_deduced);
        row("4. tuple + fold",            t_tuple);
        row("5. variant + visit",         t_variant);
        out << "\n--- Notes ---\n"
            << "* GoF vs CRTP (debug): virtual dispatch ~2x slower\n"
            << "  -- each call goes through a vtable pointer (indirect jump).\n"
            << "* GoF vs CRTP (release): gap disappears\n"
            << "  -- compiler devirtualizes when the concrete type is statically known.\n"
            << "* variant+visit (debug vs release): most dramatic result (~22x)\n"
            << "  -- debug: each visit = indexed jump through dispatch table;\n"
            << "  -- release: compiler sees a closed type set and inlines everything.\n"
            << "* tuple+fold: consistently fast -- type set fixed at compile time,\n"
            << "  fold expression expands into a sequence of direct calls.\n"
            << "* Conclusion: the static/dynamic boundary is a question of\n"
            << "  expressiveness (what can be described), not just performance.\n";
    };

    print_results(std::cout);

    std::ofstream fout("benchmark_results.txt");
    if (fout) {
        print_results(fout);
        std::cout << "\n=> results saved to benchmark_results.txt\n";
    }

    return 0;
}
