#include <ctime>
#include <iostream>

#include "AutoShopMenu.h"
#include "DemoData.h"

namespace {
autoshop::Date currentDate() {
    const std::time_t now = std::time(nullptr);
    const std::tm* local = std::localtime(&now);
    return autoshop::Date(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
}
}  // namespace

int main() {
    const autoshop::Date today = currentDate();
    autoshop::AutoShop shop = autoshop::makeDemoShop(today);
    autoshop::AutoShopMenu menu(shop, today, std::cin, std::cout);
    menu.run();
    return 0;
}
