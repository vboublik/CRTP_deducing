// Section 1-2: static (tuple+fold) participant set vs its runtime-determined limit.
// Matches the Report/Runner example referenced from [1, §2.1] in the TAAPSD paper.
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <variant>
#include <algorithm>

struct PdfReport  { void generate() const { std::cout << "Generating PDF report\n"; } };
struct HtmlReport { void generate() const { std::cout << "Generating HTML report\n"; } };
struct CsvReport  { void generate() const { std::cout << "Generating CSV report\n"; } };

// --- Section 1: compile-time fixed participant set ---
template <typename... Reports>
struct Runner {
    std::tuple<Reports...> _reports;
    explicit Runner(Reports... reports) : _reports(reports...) {}
    void run_all() {
        std::apply([](auto&... r) { 
            (r.generate(), ...); }, _reports);
    }
};

// --- Section 2: runtime-determined participant set, via type erasure ---
using ReportVariant = std::variant<PdfReport, HtmlReport, CsvReport>;

std::vector<std::string> read_user_choice() {
    // Stand-in for real input (config file, CLI args, stdin) --
    // the point is that this set is not known until the program runs.
    return {"pdf", /*"csv",*/ "html"};
}

int main() {
    std::cout << "=== Section 1: compile-time fixed set (tuple + fold) ===\n";
    Runner runner{PdfReport{}, HtmlReport{}, CsvReport{}};
    runner.run_all();

    std::cout << "\n=== Section 2: runtime-determined set (variant + visit) ===\n";
    std::vector<std::string> requested = read_user_choice();
    std::vector<ReportVariant> reports;
    if (std::find(requested.begin(), requested.end(), "pdf")  != requested.end())
        reports.push_back(PdfReport{});
    if (std::find(requested.begin(), requested.end(), "html") != requested.end())
        reports.push_back(HtmlReport{});
    if (std::find(requested.begin(), requested.end(), "csv")  != requested.end())
        reports.push_back(CsvReport{});

    for (auto& r : reports)
        std::visit([](auto& rep) { rep.generate(); }, r);

    // The commented line below is the point of Section 1's argument:
    // it cannot compile, because Reports... must be fixed at compile time.
    // std::tuple<Reports...> _reports; // <-- impossible with a runtime-chosen set
}
