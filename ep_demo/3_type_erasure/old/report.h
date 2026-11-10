// ===== СТАРА ЧАСТИНА: стирання типу (AnyReport) і std::function =====
#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

// «Інтерфейс» без базового класу: перевіряється під час компіляції (C++20)
template <typename T>
concept Report = requires(const T& r) { r.generate(); };

class AnyReport {
    struct Concept {                                   // прихований інтерфейс
        virtual ~Concept() = default;
        virtual void generate() const = 0;
    };
    template <Report T>
    struct Model final : Concept {                     // інстанціюється там, де створюється AnyReport
        T data;
        explicit Model(T val) : data(std::move(val)) {}
        void generate() const override { data.generate(); }
    };
    std::unique_ptr<Concept> self;
public:
    template <Report T>
    AnyReport(T report) : self(std::make_unique<Model<T>>(std::move(report))) {}
    void generate() const { self->generate(); }
};

struct PdfReport { void generate() const { std::cout << "[PDF]\n"; } };

// Функції, скомпільовані ЗАЗДАЛЕГІДЬ у reports.o
void run_all(const std::vector<AnyReport>& reports);
void run_actions(const std::vector<std::function<void()>>& actions);
