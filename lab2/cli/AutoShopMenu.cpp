#include "AutoShopMenu.h"

#include <istream>
#include <ostream>
#include <sstream>

namespace autoshop {

namespace {
constexpr int kEngineDemoPower = 100;
constexpr double kEngineDemoVolume = 1.6;
constexpr int kDemoTireYear = 2023;
}  // namespace

std::string toText(WorkOrderStatus status) {
    switch (status) {
        case WorkOrderStatus::Opened: return "открыт";
        case WorkOrderStatus::InProgress: return "в работе";
        case WorkOrderStatus::Completed: return "выполнен";
        case WorkOrderStatus::Cancelled: return "отменён";
    }
    return "неизвестно";
}

AutoShopMenu::AutoShopMenu(AutoShop& shop, const Date& today, std::istream& in, std::ostream& out)
    : shop_(shop), today_(today), in_(in), out_(out) {}

void AutoShopMenu::run() {
    Menu menu(shop_.getName(), in_, out_, "Выход");
    buildMenu(menu);
    menu.run();
}

void AutoShopMenu::buildMenu(Menu& menu) {
    menu.addItem("Показать клиентов и автомобили", [this] { showClients(); });
    menu.addItem("Зарегистрировать клиента", [this] { registerClient(); });
    menu.addItem("Добавить автомобиль клиенту", [this] { registerVehicle(); });
    menu.addItem("Показать услуги и остатки запчастей", [this] { showCatalogAndStock(); });
    menu.addItem("Записать на обслуживание (сегодня)", [this] { bookAppointment(); });
    buildWorkMenu(menu);
}

void AutoShopMenu::buildWorkMenu(Menu& menu) {
    menu.addItem("Открыть заказ-наряд", [this] { openOrder(); });
    menu.addItem("Добавить услугу в заказ", [this] { addServiceToOrder(); });
    menu.addItem("Добавить запчасть в заказ", [this] { addPartToOrder(); });
    menu.addItem("Начать работу (назначить механика)", [this] { startOrder(); });
    menu.addItem("Завершить работу", [this] { completeOrder(); });
    menu.addItem("Выставить счёт", [this] { issueInvoice(); });
    menu.addItem("Оплатить счёт", [this] { payInvoice(); });
    menu.addItem("Показать заказы и счета", [this] { showOrdersAndInvoices(); });
    menu.addItem("Финансовый отчёт за текущий месяц", [this] { showReport(); });
}

int AutoShopMenu::askInt(const std::string& message) { return promptValue<int>(in_, out_, message); }

Date AutoShopMenu::askDate(const std::string& message) {
    std::istringstream text(promptLine(in_, out_, message));
    int year = 0;
    int month = 0;
    int day = 0;
    char firstDash = 0;
    char secondDash = 0;
    if (!(text >> year >> firstDash >> month >> secondDash >> day) || firstDash != '-' || secondDash != '-') {
        throw std::runtime_error("Дата должна быть в формате ГГГГ-ММ-ДД");
    }
    return Date(year, month, day);
}

void AutoShopMenu::showClients() {
    for (const Mechanic& mechanic : shop_.getMechanics()) {
        out_ << "Механик #" << mechanic.getEmployeeId() << " " << mechanic.getName() << ", "
             << mechanic.getSpecialization() << "\n";
    }
    for (const Client& client : shop_.getClients()) {
        out_ << "#" << client.getId() << " " << client.getName() << ", тел. " << client.getContact().maskedPhone()
             << ", автомобилей: " << client.getVehicleVins().size() << "\n";
    }
    for (const Vehicle& vehicle : shop_.getVehicles()) {
        out_ << "  " << vehicle.getVin() << " " << vehicle.getMake() << " " << vehicle.getModel() << ", пробег "
             << vehicle.getMileage() << " км, владелец #" << vehicle.getOwnerId() << "\n";
    }
}

void AutoShopMenu::registerClient() {
    const std::string name = promptLine(in_, out_, "Имя: ");
    const Date birthDate = askDate("Дата рождения (ГГГГ-ММ-ДД): ");
    const std::string phone = promptLine(in_, out_, "Телефон: ");
    const std::string email = promptLine(in_, out_, "Email (можно пусто): ");
    const std::string street = promptLine(in_, out_, "Улица: ");
    const std::string city = promptLine(in_, out_, "Город: ");
    const std::string postalCode = promptLine(in_, out_, "Индекс: ");
    const Person person(name, birthDate, ContactInfo(phone, email), Address(street, city, postalCode));
    out_ << "Клиент зарегистрирован, номер " << shop_.registerClient(person, today_) << "\n";
}

void AutoShopMenu::registerVehicle() {
    const int clientId = askInt("Номер клиента: ");
    const std::string vin = promptLine(in_, out_, "VIN (17 символов): ");
    const std::string plate = promptLine(in_, out_, "Госномер: ");
    const std::string make = promptLine(in_, out_, "Марка: ");
    const std::string model = promptLine(in_, out_, "Модель: ");
    const int year = askInt("Год выпуска: ");
    const int mileage = askInt("Пробег, км: ");
    const Engine engine("petrol", kEngineDemoVolume, kEngineDemoPower);
    shop_.registerVehicle(clientId, Vehicle(vin, plate, make, model, year, mileage, engine,
                                            TireSet("стандарт", "all-season", 0.0, kDemoTireYear)));
    out_ << "Автомобиль добавлен\n";
}

void AutoShopMenu::showCatalogAndStock() {
    for (const ServiceType& service : shop_.getCatalog()) {
        out_ << service.getCode() << ": " << service.getName() << ", от " << service.getBasePrice().toString() << "\n";
    }
    const PartsInventory& inventory = shop_.getInventory();
    for (const std::string& sku : inventory.lowStockSkus()) {
        out_ << "Мало на складе: " << sku << " (" << inventory.quantityOf(sku) << " шт.)\n";
    }
    out_ << "Стоимость склада: " << inventory.totalValue().toString() << "\n";
}

void AutoShopMenu::bookAppointment() {
    const int clientId = askInt("Номер клиента: ");
    const std::string vin = promptLine(in_, out_, "VIN: ");
    const std::string code = promptLine(in_, out_, "Код услуги: ");
    const int hour = askInt("Час начала (8-19): ");
    const int id = shop_.bookAppointment(clientId, vin, TimeSlot(today_, hour, 1), code);
    out_ << "Запись оформлена, номер " << id << "\n";
}

void AutoShopMenu::openOrder() {
    const int clientId = askInt("Номер клиента: ");
    const std::string vin = promptLine(in_, out_, "VIN: ");
    out_ << "Заказ-наряд открыт, номер " << shop_.openWorkOrder(clientId, vin, today_) << "\n";
}

void AutoShopMenu::addServiceToOrder() {
    const int orderId = askInt("Номер заказа: ");
    shop_.addServiceToOrder(orderId, promptLine(in_, out_, "Код услуги: "));
    out_ << "Услуга добавлена\n";
}

void AutoShopMenu::addPartToOrder() {
    const int orderId = askInt("Номер заказа: ");
    const std::string sku = promptLine(in_, out_, "Артикул запчасти: ");
    shop_.addPartToOrder(orderId, sku, askInt("Количество: "));
    out_ << "Запчасть добавлена и списана со склада\n";
}

void AutoShopMenu::startOrder() {
    const int orderId = askInt("Номер заказа: ");
    shop_.startWorkOrder(orderId, askInt("Номер механика: "));
    out_ << "Работа начата\n";
}

void AutoShopMenu::completeOrder() {
    const int orderId = askInt("Номер заказа: ");
    const double hours = promptValue<double>(in_, out_, "Фактическое время, ч: ");
    shop_.completeWorkOrder(orderId, hours, today_);
    out_ << "Работа завершена, клиент уведомлён\n";
}

void AutoShopMenu::issueInvoice() {
    const int invoiceId = shop_.issueInvoice(askInt("Номер заказа: "), today_);
    out_ << "Счёт №" << invoiceId << " на " << shop_.findInvoice(invoiceId).getAmount().toString() << "\n";
}

void AutoShopMenu::payInvoice() {
    const int invoiceId = askInt("Номер счёта: ");
    const int units = askInt("Сумма (целых BYN): ");
    const std::string method = promptLine(in_, out_, "Способ (cash/card/transfer): ");
    shop_.payInvoice(invoiceId, Money::fromUnits(units), method, today_);
    out_ << "Остаток по счёту: " << shop_.findInvoice(invoiceId).outstanding().toString() << "\n";
}

void AutoShopMenu::showOrdersAndInvoices() {
    for (const WorkOrder& order : shop_.getOrders()) {
        out_ << "Заказ №" << order.getId() << " [" << toText(order.getStatus()) << "], итого "
             << order.total(shop_.getHourlyRate()).toString() << "\n";
    }
    for (const Invoice& invoice : shop_.getInvoices()) {
        out_ << "Счёт №" << invoice.getId() << ": " << invoice.getAmount().toString() << ", оплачено "
             << invoice.getPaid().toString() << "\n";
    }
}

void AutoShopMenu::showReport() {
    const Date start(today_.getYear(), today_.getMonth(), 1);
    const FinancialReport report = shop_.buildReport(start, Date(today_.getYear(), today_.getMonth(), start.daysInMonth()));
    out_ << "Выручка: " << report.getRevenue().toString() << "\nРасходы: " << report.getExpenses().toString()
         << "\nПрибыль: " << report.profit().toString() << "\n";
}

}  // namespace autoshop
