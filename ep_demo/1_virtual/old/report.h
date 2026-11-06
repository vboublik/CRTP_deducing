// ===== СТАРА ЧАСТИНА: віртуальна ієрархія =====
#pragma once
#include <iostream>
#include <memory>
#include <vector>

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
    void header() override { std::cout << "[PDF "; }
    void body()   override { std::cout << "body"; }
    void footer() override { std::cout << "]\n"; }
};

// Функція, скомпільована ЗАЗДАЛЕГІДЬ у reports.o
void run_all(const std::vector<std::unique_ptr<IReport>>& reports);
