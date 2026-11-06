#include "report.h"

void run_all(const std::vector<std::unique_ptr<IReport>>& reports) {
    for (auto& r : reports) r->generate();   // про XmlReport тут нічого не відомо
}
