#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Billing.h"
#include "Common.h"
#include "Operations.h"
#include "Parts.h"
#include "People.h"
#include "Services.h"
#include "Vehicle.h"

namespace autoshop {

inline constexpr int kWorkHoursPerMonth = 160;
inline constexpr const char* kLiftBay = "lift";
inline constexpr const char* kLoyaltyReason = "Скидка по карте лояльности";

/// Автомастерская: объединяет клиентов, автомобили, сотрудников, склад, записи, заказы и счета.
class AutoShop {
private:
    template <typename Container, typename Predicate>
    static auto& locateIn(Container& items, Predicate matches, const std::string& description) {
        auto found = std::find_if(items.begin(), items.end(), matches);
        if (found == items.end()) {
            throw NotFoundException(description);
        }
        return *found;
    }

public:
    AutoShop(std::string name, const Address& address, int bayCount, const Money& hourlyRate)
        : name_(std::move(name)), address_(address), hourlyRate_(hourlyRate), appointments_(bayCount),
          inventory_(name_ + ": склад") {
        for (int number = 1; number <= bayCount; ++number) {
            bays_.emplace_back(number, kLiftBay);
        }
    }

    const std::string& getName() const { return name_; }
    const Address& getAddress() const { return address_; }
    const Money& getHourlyRate() const { return hourlyRate_; }
    const std::vector<Client>& getClients() const { return clients_; }
    const std::vector<Vehicle>& getVehicles() const { return vehicles_; }
    const std::vector<Mechanic>& getMechanics() const { return mechanics_; }
    const std::vector<WorkOrder>& getOrders() const { return orders_; }
    const std::vector<Invoice>& getInvoices() const { return invoices_; }
    const std::vector<ServiceType>& getCatalog() const { return catalog_; }
    const std::vector<WorkshopBay>& getBays() const { return bays_; }
    const PartsInventory& getInventory() const { return inventory_; }
    const AppointmentBook& getAppointments() const { return appointments_; }
    const NotificationService& getNotifications() const { return notifier_; }

    int registerClient(const Person& person, const Date& registeredOn) {
        const int id = nextId();
        clients_.emplace_back(id, person, registeredOn);
        loyaltyCards_.emplace_back(clients_.back(), registeredOn);
        return id;
    }
    void registerVehicle(int clientId, const Vehicle& vehicle) {
        Client& client = locateClient(clientId);
        if (hasVehicle(vehicle.getVin())) {
            throw InvalidVehicleDataException("VIN уже зарегистрирован: " + vehicle.getVin());
        }
        vehicles_.push_back(vehicle);
        client.registerVehicle(vehicles_.back());
    }
    int hireMechanic(const Person& person, const std::string& specialization, const Money& hourlyRate,
                     const Date& hiredOn) {
        const int id = nextId();
        const Employee employee(id, person, "Механик", hiredOn, hourlyRate.multiply(kWorkHoursPerMonth));
        mechanics_.emplace_back(employee, specialization, hourlyRate);
        return id;
    }
    void addServiceType(const ServiceType& service) { catalog_.push_back(service); }
    void addPart(const Part& part, int quantity, int minLevel) { inventory_.registerPart(part, quantity, minLevel); }
    void recordExpense(const ExpenseRecord& expense) { expenses_.push_back(expense); }

    int bookAppointment(int clientId, const std::string& vin, const TimeSlot& slot, const std::string& serviceCode) {
        const Client& client = findClient(clientId);
        const Appointment appointment(nextId(), client, findVehicle(vin), slot, findService(serviceCode));
        appointments_.book(appointment);
        notifier_.notifyAppointment(client, appointment);
        return appointment.getId();
    }
    int openWorkOrder(int clientId, const std::string& vin, const Date& openedOn) {
        orders_.emplace_back(nextId(), findClient(clientId), findVehicle(vin), openedOn);
        return orders_.back().getId();
    }
    void addServiceToOrder(int orderId, const std::string& serviceCode) {
        locateOrder(orderId).addService(findService(serviceCode));
    }
    void addPartToOrder(int orderId, const std::string& sku, int quantity) {
        const OrderLine line(inventory_.find(sku).getPart(), quantity);
        if (!inventory_.isAvailable(line)) {
            throw OutOfStockException(line.getPart().getName());
        }
        locateOrder(orderId).addPart(line);
        inventory_.issue(line);
    }
    void startWorkOrder(int orderId, int mechanicId) {
        WorkOrder& order = locateOrder(orderId);
        WorkshopBay& bay = locateFreeBay();
        order.assignMechanic(findMechanic(mechanicId));
        order.start();
        bay.occupy(order);
    }
    void completeWorkOrder(int orderId, double actualHours, const Date& date) {
        WorkOrder& order = locateOrder(orderId);
        order.complete(actualHours);
        for (WorkshopBay& bay : bays_) {
            if (bay.getOccupiedByOrderId() == orderId) {
                bay.release();
            }
        }
        notifier_.notifyWorkReady(findClient(order.getClientId()), order, date);
    }
    int issueInvoice(int orderId, const Date& issuedOn) {
        const WorkOrder& order = findOrder(orderId);
        Invoice invoice(nextId(), order, order.total(hourlyRate_), issuedOn);
        const double loyaltyPercent = locateCard(order.getClientId()).discountPercent();
        if (loyaltyPercent > 0.0) {
            invoice.applyDiscount(Discount(loyaltyPercent, kLoyaltyReason, issuedOn, issuedOn), issuedOn);
        }
        notifier_.notifyInvoice(findClient(order.getClientId()), invoice);
        invoices_.push_back(invoice);
        return invoice.getId();
    }
    int payInvoice(int invoiceId, const Money& amount, const std::string& method, const Date& paidOn) {
        Invoice& invoice = locateInvoice(invoiceId);
        const Payment payment(nextId(), invoice, amount, method, paidOn);
        invoice.registerPayment(amount);
        locateClient(invoice.getClientId()).recordSpending(amount);
        locateCard(invoice.getClientId()).earn(amount);
        return payment.getId();
    }
    FinancialReport buildReport(const Date& start, const Date& end) const {
        FinancialReport report(start, end);
        for (const Invoice& invoice : invoices_) {
            report.addInvoice(invoice);
        }
        for (const ExpenseRecord& expense : expenses_) {
            report.addExpense(expense);
        }
        return report;
    }
    const Client& findClient(int id) const {
        return locateIn(clients_, [id](const Client& c) { return c.getId() == id; }, "клиент " + std::to_string(id));
    }
    const Vehicle& findVehicle(const std::string& vin) const {
        return locateIn(vehicles_, [&vin](const Vehicle& v) { return v.getVin() == vin; }, "автомобиль " + vin);
    }
    const Mechanic& findMechanic(int id) const {
        return locateIn(mechanics_, [id](const Mechanic& m) { return m.getEmployeeId() == id; },
                        "механик " + std::to_string(id));
    }
    const WorkOrder& findOrder(int id) const {
        return locateIn(orders_, [id](const WorkOrder& o) { return o.getId() == id; }, "заказ-наряд " + std::to_string(id));
    }
    const Invoice& findInvoice(int id) const {
        return locateIn(invoices_, [id](const Invoice& i) { return i.getId() == id; }, "счёт " + std::to_string(id));
    }
    const ServiceType& findService(const std::string& code) const {
        return locateIn(catalog_, [&code](const ServiceType& s) { return s.getCode() == code; }, "услуга " + code);
    }
    bool hasVehicle(const std::string& vin) const {
        return std::any_of(vehicles_.begin(), vehicles_.end(), [&vin](const Vehicle& v) { return v.getVin() == vin; });
    }

private:
    int nextId() { return nextId_++; }
    Client& locateClient(int id) {
        return locateIn(clients_, [id](const Client& c) { return c.getId() == id; }, "клиент " + std::to_string(id));
    }
    WorkOrder& locateOrder(int id) {
        return locateIn(orders_, [id](const WorkOrder& o) { return o.getId() == id; }, "заказ-наряд " + std::to_string(id));
    }
    Invoice& locateInvoice(int id) {
        return locateIn(invoices_, [id](const Invoice& i) { return i.getId() == id; }, "счёт " + std::to_string(id));
    }
    LoyaltyCard& locateCard(int clientId) {
        return locateIn(loyaltyCards_, [clientId](const LoyaltyCard& c) { return c.getClientId() == clientId; },
                        "карта клиента " + std::to_string(clientId));
    }
    WorkshopBay& locateFreeBay() {
        auto found = std::find_if(bays_.begin(), bays_.end(), [](const WorkshopBay& b) { return b.isFree(); });
        if (found == bays_.end()) {
            throw BayNotAvailableException("нет свободных постов");
        }
        return *found;
    }

    std::string name_;
    Address address_;
    Money hourlyRate_;
    int nextId_ = 1;
    std::vector<Client> clients_;
    std::vector<Vehicle> vehicles_;
    std::vector<Mechanic> mechanics_;
    std::vector<WorkOrder> orders_;
    std::vector<Invoice> invoices_;
    std::vector<ExpenseRecord> expenses_;
    std::vector<ServiceType> catalog_;
    std::vector<LoyaltyCard> loyaltyCards_;
    std::vector<WorkshopBay> bays_;
    AppointmentBook appointments_;
    PartsInventory inventory_;
    NotificationService notifier_;
};

}  // namespace autoshop
