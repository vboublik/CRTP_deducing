// ===== СТАРА ЧАСТИНА: кортежна інфраструктура, операції як зовнішні перевантаження =====
// Шаблони не компілюються в .o, доки їх не інстанціюють, тож «старий» код тут — лише заголовок.
#pragma once
#include <iostream>
#include <tuple>

namespace rep {
    struct PdfReport  { int pages = 3; void generate() const { std::cout << "[PDF]\n"; } };
    struct HtmlReport { int links = 5; void generate() const { std::cout << "<html/>\n"; } };

    // наявна операція: набір зовнішніх перевантажень, по одному на тип
    inline int size(const PdfReport& r)  { return r.pages; }
    inline int size(const HtmlReport& r) { return r.links; }
}

template <typename... Reports>
struct Runner {
    std::tuple<Reports...> reports;
    explicit Runner(Reports... r) : reports(r...) {}
    void run_all() { std::apply([](auto&... r) { (r.generate(), ...); }, reports); }
    template <class Op>
    int sum(Op op) { return std::apply([&](auto&... r) { return (op(r) + ...); }, reports); }
};
