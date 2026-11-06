// ===== НОВИЙ ПРОЕКТ (б): додаємо НОВИЙ ТИП =====
// Щоб додати XmlReport, доводиться змінити сам тип Report (WITH_XML).
// Стара reports.o скомпільована зі СТАРИМ Report, тому не підходить:
// збирання з нею завершиться помилкою компонування (undefined reference).
#define WITH_XML
#include "../old/report.h"

int main() {
    std::vector<Report> reports{ PdfReport{}, XmlReport{} };
    run_all(reports);
}
