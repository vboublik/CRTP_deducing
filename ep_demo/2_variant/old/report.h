// ===== СТАРА ЧАСТИНА: закрита сума std::variant =====
#pragma once
#include <iostream>
#include <variant>
#include <vector>

struct PdfReport  { void generate() const { std::cout << "[PDF]\n"; } };
struct HtmlReport { void generate() const { std::cout << "<html/>\n"; } };
struct CsvReport  { void generate() const { std::cout << "a;b;c\n"; } };

#ifdef WITH_XML   // спроба додати новий тип у ЗАКРИТИЙ перелік
struct XmlReport  { void generate() const { std::cout << "<xml/>\n"; } };
using Report = std::variant<PdfReport, HtmlReport, CsvReport, XmlReport>;
#else
using Report = std::variant<PdfReport, HtmlReport, CsvReport>;
#endif

// Функція, скомпільована ЗАЗДАЛЕГІДЬ у reports.o
void run_all(const std::vector<Report>& reports);
