#pragma once

#include <iosfwd>
#include <string>

#include "Menu.h"
#include "autoshop/AutoShop.h"

namespace autoshop {

/// Консольное меню автомастерской. Логика предметной области остаётся в классах из include/autoshop.
class AutoShopMenu {
public:
    /// @param shop мастерская, с которой работает меню
    /// @param today «сегодняшняя» дата (передаётся снаружи, чтобы меню было тестируемым)
    /// @param in поток ввода
    /// @param out поток вывода
    AutoShopMenu(AutoShop& shop, const Date& today, std::istream& in, std::ostream& out);

    /// Запускает главное меню.
    void run();

private:
    void buildMenu(Menu& menu);
    void buildWorkMenu(Menu& menu);
    void showClients();
    void registerClient();
    void registerVehicle();
    void showCatalogAndStock();
    void bookAppointment();
    void openOrder();
    void addServiceToOrder();
    void addPartToOrder();
    void startOrder();
    void completeOrder();
    void issueInvoice();
    void payInvoice();
    void showOrdersAndInvoices();
    void showReport();
    int askInt(const std::string& message);
    Date askDate(const std::string& message);

    AutoShop& shop_;
    Date today_;
    std::istream& in_;
    std::ostream& out_;
};

/// @return текстовое название статуса заказ-наряда
std::string toText(WorkOrderStatus status);

}  // namespace autoshop
