#include "report.h"

void run_all(const std::vector<AnyReport>& reports) {
    for (const auto& r : reports) r.generate();      // про нові типи тут нічого не відомо
}

void run_actions(const std::vector<std::function<void()>>& actions) {
    for (const auto& a : actions) a();
}
