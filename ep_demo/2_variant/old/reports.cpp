#include "report.h"

void run_all(const std::vector<Report>& reports) {
    for (auto& r : reports)
        std::visit([](auto& x) { x.generate(); }, r);
}
