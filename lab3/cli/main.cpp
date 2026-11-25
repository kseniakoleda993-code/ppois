#include <ctime>
#include <iostream>

#include "DemoData.h"
#include "StudentSystemMenu.h"

namespace {
studis::Date currentDate() {
    const std::time_t now = std::time(nullptr);
    const std::tm* local = std::localtime(&now);
    return studis::Date(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
}
}  // namespace

int main() {
    const studis::Date today = currentDate();
    studis::StudentInformationSystem system = studis::makeDemoSystem(today);
    studis::StudentSystemMenu menu(system, today, std::cin, std::cout);
    menu.run();
    return 0;
}
