// ===== НОВИЙ ПРОЕКТ: і новий тип, і нова операція — без зміни report.h =====
// Ціна: склад відомий під час компіляції, усе перекомпілюється.
#include "../old/report.h"

namespace rep {
    // нова операція: новий набір перевантажень, типи не змінюються
    inline int weight(const PdfReport& r)  { return r.pages * 10; }
    inline int weight(const HtmlReport& r) { return r.links; }

    // новий тип: тип + перевантаження наявних операцій, теж лише новий код
    struct XmlReport { int nodes = 7; void generate() const { std::cout << "<xml/>\n"; } };
    inline int size(const XmlReport& r)   { return r.nodes; }
    inline int weight(const XmlReport& r) { return r.nodes * 2; }
}

int main() {
    Runner runner{rep::PdfReport{}, rep::HtmlReport{}, rep::XmlReport{}};
    runner.run_all();
    std::cout << "size = "   << runner.sum([](auto& r) { return size(r); })
              << ", weight = " << runner.sum([](auto& r) { return weight(r); }) << '\n';
}
