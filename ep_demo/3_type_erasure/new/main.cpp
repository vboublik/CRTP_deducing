// ===== НОВИЙ ПРОЕКТ: новий тип для обох форм стирання =====
// reports.o НЕ перекомпілюється, лише підключається при збиранні.
#include "../old/report.h"

struct JsonReport { void generate() const { std::cout << "{json}\n"; } };   // новий тип
struct XmlReport  { void to_xml() const   { std::cout << "<xml/>\n"; } };   // навіть інша назва методу

int main() {
    std::vector<AnyReport> reports;
    reports.emplace_back(PdfReport{});
    reports.emplace_back(JsonReport{});
    run_all(reports);                                 // Model<JsonReport> інстанційовано тут

    std::vector<std::function<void()>> actions{
        [] { XmlReport{}.to_xml(); } };               // лямбда-адаптер
    run_actions(actions);
}
