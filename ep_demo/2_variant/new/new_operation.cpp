// ===== НОВИЙ ПРОЕКТ (а): додаємо НОВУ ОПЕРАЦІЮ =====
// reports.o НЕ перекомпілюється; типи не змінюються.
#include "../old/report.h"

struct Validate {                      // нова операція: лише новий visitor
    bool operator()(const PdfReport&)  const { return true; }
    bool operator()(const HtmlReport&) const { return true; }
    bool operator()(const CsvReport&)  const { return false; }
};

int main() {
    std::vector<Report> reports{ PdfReport{}, CsvReport{}, HtmlReport{} };
    run_all(reports);                                   // стара функція з reports.o
    for (auto& r : reports)
        std::cout << "valid: " << std::visit(Validate{}, r) << '\n';  // нова операція
}
