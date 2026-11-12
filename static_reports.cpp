// tuple + fold vs variant + visit: the example from the TAAPSD 2026 theses,
// extended with the two axes of the Expression Problem (talk slides 10-12).
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <variant>

void doPdf()  { std::cout << "Generating PDF report\n"; }
void doHtml() { std::cout << "Generating HTML report\n"; }
void doCsv()  { std::cout << "Generating CSV report\n"; }

struct PdfReport  { void generate() const { doPdf(); } };
struct HtmlReport { void generate() const { doHtml(); } };
struct CsvReport  { void generate() const { doCsv(); } };
// struct XmlReport { void generate() const { std::cout << "Generating XML report\n"; } };  // slide 12: new type

// --- Section 1 (slide 7): compile-time fixed participant set ---
template <typename... Reports>
struct Runner {
    std::tuple<Reports...> _reports;
    explicit Runner(Reports... reports) : _reports(reports...) {}
    void run_all() {
        std::apply([](auto&... r) {
            (r.generate(), ...);
        }, _reports);
    }
};

// --- Section 2 (slide 10): runtime-determined participant set, closed sum (variant) ---
using ReportVariant = std::variant<PdfReport, HtmlReport, CsvReport /*, XmlReport */>;

struct Validate {                        // an operation as a visitor: checks report content
    bool operator()(const PdfReport&)  const { return true; }
    bool operator()(const HtmlReport&) const { return true; }
    bool operator()(const CsvReport&)  const { return true; }
    // bool operator()(const XmlReport&) const { return true; }   // slide 12: branch for the new type
};

std::vector<ReportVariant> read_user_choice() {
    // Stand-in for real input (config file, CLI args, stdin) --
    // the point is that count, order and repeats are known only at runtime.
    std::vector<std::string> names = {"pdf", "csv", "html", "pdf"};
    std::vector<ReportVariant> reports;
    for (const auto& name : names) {
        if      (name == "pdf")  reports.push_back(PdfReport{});
        else if (name == "html") reports.push_back(HtmlReport{});
        else if (name == "csv")  reports.push_back(CsvReport{});
    }
    return reports;
}

// --- Section 3 (slide 11): a new operation is a new visitor; the report types are untouched ---
struct Save {
    void operator()(const PdfReport&)  const { std::cout << "Saving report.pdf\n"; }
    void operator()(const HtmlReport&) const { std::cout << "Saving report.html\n"; }
    void operator()(const CsvReport&)  const { std::cout << "Saving report.csv\n"; }
    // void operator()(const XmlReport&) const { std::cout << "Saving report.xml\n"; }  // slide 12
};
// Slide 12: a new type in ReportVariant needs a branch in EVERY such visitor
// (Validate and Save); otherwise std::visit does not compile.

int main() {
    std::cout << "=== Slide 7: compile-time fixed set (tuple + fold) ===\n";
    Runner runner{PdfReport{}, HtmlReport{}, CsvReport{}};
    runner.run_all();

    // The line below is the point of the tuple's limit:
    // it cannot compile, because Reports... must be fixed at compile time.
    // std::tuple<Reports...> _reports; // <-- impossible with a runtime-chosen set

    std::cout << "\n=== Slide 10: runtime-determined set (variant + visit) ===\n";
    std::vector<ReportVariant> reports = read_user_choice();
    for (auto& r : reports)
        if (std::visit(Validate{}, r))
            std::visit([](const auto& rep) { rep.generate(); }, r);
        else
            std::cout << "invalid report\n";

    std::cout << "\n=== Slide 11: new operation (visitor) ===\n";
    for (const auto& r : reports)
        std::visit(Save{}, r);
}
