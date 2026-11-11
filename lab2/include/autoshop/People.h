#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Parts.h"
#include "Services.h"
#include "Vehicle.h"

namespace autoshop {

inline constexpr int kAdultAge = 18;

/// Квалификационный сертификат механика.
class Certificate {
public:
    Certificate(std::string name, const Date& issuedOn, const Date& validUntil)
        : name_(std::move(name)), issuedOn_(issuedOn), validUntil_(validUntil) {}

    const std::string& getName() const { return name_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    const Date& getValidUntil() const { return validUntil_; }
    bool isValidOn(const Date& date) const { return !date.isBefore(issuedOn_) && !validUntil_.isBefore(date); }
    int daysLeft(const Date& date) const { return std::max(0, date.daysUntil(validUntil_)); }

private:
    std::string name_;
    Date issuedOn_;
    Date validUntil_;
};

/// Человек: общие данные клиентов и сотрудников.
class Person {
public:
    Person(std::string name, const Date& birthDate, const ContactInfo& contact, const Address& address)
        : name_(std::move(name)), birthDate_(birthDate), contact_(contact), address_(address) {}
    virtual ~Person() = default;

    const std::string& getName() const { return name_; }
    const Date& getBirthDate() const { return birthDate_; }
    const ContactInfo& getContact() const { return contact_; }
    const Address& getAddress() const { return address_; }
    int ageOn(const Date& date) const {
        const bool birthdayPassed = std::make_pair(date.getMonth(), date.getDay()) >=
                                    std::make_pair(birthDate_.getMonth(), birthDate_.getDay());
        return date.getYear() - birthDate_.getYear() - (birthdayPassed ? 0 : 1);
    }
    bool isAdultOn(const Date& date) const { return ageOn(date) >= kAdultAge; }
    void moveTo(const Address& newAddress) { address_ = newAddress; }
    void updateContact(const ContactInfo& newContact) { contact_ = newContact; }

private:
    std::string name_;
    Date birthDate_;
    ContactInfo contact_;
    Address address_;
};

inline const Money kRegularCustomerThreshold = Money::fromUnits(1000);

/// Клиент автомастерской.
class Client : public Person {
public:
    Client(int id, const Person& person, const Date& registeredOn) : Person(person), id_(id), registeredOn_(registeredOn) {
        const bool emailIsBad = !getContact().getEmail().empty() && !getContact().hasValidEmail();
        if (getName().empty() || !getContact().hasValidPhone() || emailIsBad) {
            throw InvalidClientDataException(getName());
        }
        if (!isAdultOn(registeredOn_)) {
            throw InvalidClientDataException("клиент должен быть совершеннолетним");
        }
    }

    int getId() const { return id_; }
    const Date& getRegisteredOn() const { return registeredOn_; }
    const std::vector<std::string>& getVehicleVins() const { return vehicleVins_; }
    const Money& getTotalSpent() const { return totalSpent_; }
    void registerVehicle(Vehicle& vehicle) {
        if (std::find(vehicleVins_.begin(), vehicleVins_.end(), vehicle.getVin()) != vehicleVins_.end()) {
            throw InvalidVehicleDataException("автомобиль уже зарегистрирован: " + vehicle.getVin());
        }
        vehicleVins_.push_back(vehicle.getVin());
        vehicle.assignOwnerId(id_);
    }
    bool ownsVehicle(const Vehicle& vehicle) const {
        return std::find(vehicleVins_.begin(), vehicleVins_.end(), vehicle.getVin()) != vehicleVins_.end();
    }
    void releaseVehicle(const Vehicle& vehicle) {
        vehicleVins_.erase(std::remove(vehicleVins_.begin(), vehicleVins_.end(), vehicle.getVin()), vehicleVins_.end());
    }
    void recordSpending(const Money& amount) { totalSpent_ = totalSpent_.plus(amount); }
    bool isRegularCustomer() const { return totalSpent_.isGreaterOrEqual(kRegularCustomerThreshold); }

private:
    int id_;
    Date registeredOn_;
    std::vector<std::string> vehicleVins_;
    Money totalSpent_;
};

/// Сотрудник автомастерской.
class Employee : public Person {
public:
    Employee(int employeeId, const Person& person, std::string position, const Date& hireDate,
             const Money& monthlySalary)
        : Person(person), employeeId_(employeeId), position_(std::move(position)), hireDate_(hireDate),
          monthlySalary_(monthlySalary) {
        if (monthlySalary_.isNegative()) {
            throw InvalidMoneyOperationException("отрицательная зарплата");
        }
    }

    int getEmployeeId() const { return employeeId_; }
    const std::string& getPosition() const { return position_; }
    const Date& getHireDate() const { return hireDate_; }
    const Money& getMonthlySalary() const { return monthlySalary_; }
    int yearsOfServiceOn(const Date& date) const { return hireDate_.daysUntil(date) / kDaysPerYear; }
    bool isEligibleForBonusOn(const Date& date) const { return yearsOfServiceOn(date) >= 1; }
    void raiseSalary(double percent) { monthlySalary_ = monthlySalary_.plus(monthlySalary_.applyPercent(percent)); }

private:
    int employeeId_;
    std::string position_;
    Date hireDate_;
    Money monthlySalary_;
};

/// Механик, выполняющий ремонт.
class Mechanic : public Employee {
public:
    Mechanic(const Employee& employee, std::string specialization, const Money& hourlyRate)
        : Employee(employee), specialization_(std::move(specialization)), hourlyRate_(hourlyRate) {}

    const std::string& getSpecialization() const { return specialization_; }
    const Money& getHourlyRate() const { return hourlyRate_; }
    void addCertificate(const Certificate& certificate) { certificates_.push_back(certificate); }
    bool hasValidCertificate(const std::string& skill, const Date& date) const {
        return std::any_of(certificates_.begin(), certificates_.end(),
                           [&](const Certificate& c) { return c.getName() == skill && c.isValidOn(date); });
    }
    bool canHandle(const ServiceType& service, const Date& date) const {
        return service.requiresSkill(specialization_) || hasValidCertificate(service.getRequiredSkill(), date);
    }
    bool isAvailable(const TimeSlot& slot) const {
        return std::none_of(busySlots_.begin(), busySlots_.end(), [&](const TimeSlot& busy) { return busy.overlaps(slot); });
    }
    void reserve(const TimeSlot& slot) {
        if (!isAvailable(slot)) {
            throw MechanicNotAvailableException(getName());
        }
        busySlots_.push_back(slot);
    }
    Money laborCost(double hours) const { return hourlyRate_.multiply(hours); }

private:
    std::string specialization_;
    Money hourlyRate_;
    std::vector<Certificate> certificates_;
    std::vector<TimeSlot> busySlots_;
};

/// Приёмщик: общается с клиентами и рекомендует услуги.
class ServiceAdvisor : public Employee {
public:
    ServiceAdvisor(const Employee& employee, double commissionPercent)
        : Employee(employee), commissionPercent_(commissionPercent) {}

    int getCustomersServed() const { return customersServed_; }
    double getCommissionPercent() const { return commissionPercent_; }
    void recordServedClient(const Client& client) {
        if (client.getId() > 0) {
            ++customersServed_;
        }
    }
    Money calculateCommission(const Money& saleAmount) const { return saleAmount.applyPercent(commissionPercent_); }
    std::vector<std::string> recommendServices(const std::vector<ServicePlan>& plans, const Vehicle& vehicle) const {
        std::vector<std::string> names;
        for (const ServicePlan& plan : plans) {
            if (plan.isDue(vehicle)) {
                names.push_back(plan.getServiceName());
            }
        }
        return names;
    }

private:
    int customersServed_ = 0;
    double commissionPercent_;
};

/// Руководитель мастерской: утверждает скидки и возвраты.
class Manager : public Employee {
public:
    Manager(const Employee& employee, double discountLimitPercent, const Money& refundLimit)
        : Employee(employee), discountLimitPercent_(discountLimitPercent), refundLimit_(refundLimit) {}

    double getDiscountLimitPercent() const { return discountLimitPercent_; }
    const Money& getRefundLimit() const { return refundLimit_; }
    int getApprovedCount() const { return approvedCount_; }
    bool canApprove(const Discount& discount) const { return discount.getPercent() <= discountLimitPercent_; }
    void approveDiscount(const Discount& discount) {
        if (!canApprove(discount)) {
            throw DiscountLimitExceededException(discount.getReason());
        }
        ++approvedCount_;
    }
    void authorizeRefund(const Money& amount) const {
        if (!refundLimit_.isGreaterOrEqual(amount)) {
            throw InsufficientFundsException("сумма возврата превышает лимит менеджера");
        }
    }
    void raiseLimit(double percent) { discountLimitPercent_ = std::min(kMaxDiscountPercent, discountLimitPercent_ + percent); }

private:
    double discountLimitPercent_;
    Money refundLimit_;
    int approvedCount_ = 0;
};

/// Кассир: принимает оплату и работает с кассой.
class Cashier : public Employee {
public:
    Cashier(const Employee& employee, const Money& openingCash) : Employee(employee), cashDrawer_(openingCash) {}

    const Money& getCashDrawer() const { return cashDrawer_; }
    int getReceiptsIssued() const { return receiptsIssued_; }
    void acceptCash(const Money& amount) { cashDrawer_ = cashDrawer_.plus(amount); }
    Money giveChange(const Money& due, const Money& paid) {
        if (!paid.isGreaterOrEqual(due)) {
            throw InsufficientFundsException("внесено меньше, чем к оплате");
        }
        const Money change = paid.minus(due);
        cashDrawer_ = cashDrawer_.plus(due);
        return change;
    }
    void withdraw(const Money& amount) {
        if (!cashDrawer_.isGreaterOrEqual(amount)) {
            throw InsufficientFundsException("в кассе недостаточно наличных");
        }
        cashDrawer_ = cashDrawer_.minus(amount);
    }
    void registerReceipt() { ++receiptsIssued_; }

private:
    Money cashDrawer_;
    int receiptsIssued_ = 0;
};

/// Кладовщик: принимает поставки и выдаёт запчасти.
class WarehouseKeeper : public Employee {
public:
    WarehouseKeeper(const Employee& employee, std::string zone) : Employee(employee), zone_(std::move(zone)) {}

    const std::string& getZone() const { return zone_; }
    int getOperationsCount() const { return operationsCount_; }
    void receiveDelivery(PartsInventory& inventory, PurchaseOrder& order) {
        order.markReceived();
        for (const OrderLine& line : order.getLines()) {
            inventory.receive(line.getPart().getSku(), line.getQuantity());
            ++operationsCount_;
        }
    }
    void issueParts(PartsInventory& inventory, const OrderLine& line) {
        inventory.issue(line);
        ++operationsCount_;
    }
    std::vector<std::string> buildReorderList(const PartsInventory& inventory) const { return inventory.lowStockSkus(); }

private:
    std::string zone_;
    int operationsCount_ = 0;
};

}  // namespace autoshop
