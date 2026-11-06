// ===== НОВИЙ ПРОЕКТ: додаємо НОВИЙ ТИП =====
// reports.o НЕ перекомпілюється, лише підключається при збиранні.
#include "../old/report.h"

struct XmlReport : IReport {          // новий тип: лише новий код
private:
    void header() override { std::cout << "<xml>"; }
    void body()   override { std::cout << "body"; }
    void footer() override { std::cout << "</xml>\n"; }
};

int main() {
    std::vector<std::unique_ptr<IReport>> reports;
    reports.push_back(std::make_unique<PdfReport>());
    reports.push_back(std::make_unique<XmlReport>());
    run_all(reports);   // стара скомпільована функція викликає новий тип через vtable
}
