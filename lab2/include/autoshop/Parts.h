#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Vehicle.h"

namespace autoshop {

/// Запчасть или расходный материал.
class Part {
public:
    Part(std::string sku, std::string name, std::string category, const Money& price,
         std::vector<std::string> compatibleMakes = {})
        : sku_(std::move(sku)), name_(std::move(name)), category_(std::move(category)), price_(price),
          compatibleMakes_(std::move(compatibleMakes)) {
        if (sku_.empty() || name_.empty() || price_.isNegative()) {
            throw InvalidPartDataException(sku_);
        }
    }

    const std::string& getSku() const { return sku_; }
    const std::string& getName() const { return name_; }
    const std::string& getCategory() const { return category_; }
    const Money& getPrice() const { return price_; }
    const std::vector<std::string>& getCompatibleMakes() const { return compatibleMakes_; }
    Money priceWithMarkup(double percent) const { return price_.plus(price_.applyPercent(percent)); }
    bool isCompatibleWith(const Vehicle& vehicle) const {
        return compatibleMakes_.empty() ||
               std::find(compatibleMakes_.begin(), compatibleMakes_.end(), vehicle.getMake()) != compatibleMakes_.end();
    }

private:
    std::string sku_;
    std::string name_;
    std::string category_;
    Money price_;
    std::vector<std::string> compatibleMakes_;
};

inline constexpr double kReliableSupplierRating = 4.0;
inline constexpr double kMaxSupplierRating = 5.0;

/// Поставщик запчастей.
class Supplier {
public:
    Supplier(std::string name, const ContactInfo& contact, int leadTimeDays, double rating)
        : name_(std::move(name)), contact_(contact), leadTimeDays_(leadTimeDays), rating_(rating) {}

    const std::string& getName() const { return name_; }
    const ContactInfo& getContact() const { return contact_; }
    int getLeadTimeDays() const { return leadTimeDays_; }
    double getRating() const { return rating_; }
    Date estimateDelivery(const Date& orderedOn) const { return orderedOn.plusDays(leadTimeDays_); }
    bool isReliable() const { return rating_ >= kReliableSupplierRating; }
    void rate(double newRating) {
        if (newRating < 0.0 || newRating > kMaxSupplierRating) {
            throw InvalidPartDataException("оценка поставщика вне диапазона");
        }
        rating_ = newRating;
    }

private:
    std::string name_;
    ContactInfo contact_;
    int leadTimeDays_;
    double rating_;
};

/// Строка заказа: запчасть и количество.
class OrderLine {
public:
    OrderLine(const Part& part, int quantity) : part_(part), quantity_(quantity) {
        if (quantity_ <= 0) {
            throw InvalidPartDataException("количество должно быть положительным");
        }
    }

    const Part& getPart() const { return part_; }
    int getQuantity() const { return quantity_; }
    Money lineTotal() const { return part_.getPrice().multiply(quantity_); }
    void increase(int amount) {
        if (amount <= 0) {
            throw InvalidPartDataException("увеличение должно быть положительным");
        }
        quantity_ += amount;
    }

private:
    Part part_;
    int quantity_;
};

/// Складская запись об одной запчасти.
class StockRecord {
public:
    StockRecord(const Part& part, int quantity, int minLevel, std::string shelf)
        : part_(part), quantity_(quantity), minLevel_(minLevel), shelf_(std::move(shelf)) {}

    const Part& getPart() const { return part_; }
    int getQuantity() const { return quantity_; }
    int getMinLevel() const { return minLevel_; }
    const std::string& getShelf() const { return shelf_; }
    bool isLow() const { return quantity_ <= minLevel_; }
    void add(int amount) { quantity_ += amount; }
    void remove(int amount) {
        if (amount > quantity_) {
            throw OutOfStockException(part_.getName());
        }
        quantity_ -= amount;
    }
    int restockAmount() const { return isLow() ? minLevel_ * 2 - quantity_ : 0; }

private:
    Part part_;
    int quantity_;
    int minLevel_;
    std::string shelf_;
};

/// Склад запчастей автомастерской.
class PartsInventory {
public:
    explicit PartsInventory(std::string location) : location_(std::move(location)) {}

    const std::string& getLocation() const { return location_; }
    void registerPart(const Part& part, int quantity, int minLevel) {
        auto found = records_.find(part.getSku());
        if (found != records_.end()) {
            found->second.add(quantity);
            return;
        }
        records_.emplace(part.getSku(), StockRecord(part, quantity, minLevel, location_));
    }
    void receive(const std::string& sku, int quantity) { locate(sku).add(quantity); }
    bool isAvailable(const OrderLine& line) const {
        auto found = records_.find(line.getPart().getSku());
        return found != records_.end() && found->second.getQuantity() >= line.getQuantity();
    }
    void issue(const OrderLine& line) { locate(line.getPart().getSku()).remove(line.getQuantity()); }
    const StockRecord& find(const std::string& sku) const {
        auto found = records_.find(sku);
        if (found == records_.end()) {
            throw NotFoundException("запчасть " + sku);
        }
        return found->second;
    }
    int quantityOf(const std::string& sku) const { return find(sku).getQuantity(); }
    std::vector<std::string> lowStockSkus() const {
        std::vector<std::string> result;
        for (const auto& entry : records_) {
            if (entry.second.isLow()) {
                result.push_back(entry.first);
            }
        }
        return result;
    }
    Money totalValue() const {
        Money total;
        for (const auto& entry : records_) {
            total = total.plus(entry.second.getPart().getPrice().multiply(entry.second.getQuantity()));
        }
        return total;
    }

private:
    StockRecord& locate(const std::string& sku) {
        auto found = records_.find(sku);
        if (found == records_.end()) {
            throw NotFoundException("запчасть " + sku);
        }
        return found->second;
    }

    std::map<std::string, StockRecord> records_;
    std::string location_;
};

enum class PurchaseStatus { Draft, Sent, Received, Cancelled };

/// Заказ поставщику на пополнение склада.
class PurchaseOrder {
public:
    PurchaseOrder(int id, const Supplier& supplier, const Date& orderedOn)
        : id_(id), supplier_(supplier), orderedOn_(orderedOn) {}

    int getId() const { return id_; }
    const Supplier& getSupplier() const { return supplier_; }
    const std::vector<OrderLine>& getLines() const { return lines_; }
    const Date& getOrderedOn() const { return orderedOn_; }
    PurchaseStatus getStatus() const { return status_; }
    void addLine(const OrderLine& line) {
        requireStatus(PurchaseStatus::Draft, "добавление позиции");
        lines_.push_back(line);
    }
    Money total() const {
        Money sum;
        for (const OrderLine& line : lines_) {
            sum = sum.plus(line.lineTotal());
        }
        return sum;
    }
    Date expectedDelivery() const { return supplier_.estimateDelivery(orderedOn_); }
    void send() {
        requireStatus(PurchaseStatus::Draft, "отправка");
        status_ = PurchaseStatus::Sent;
    }
    void markReceived() {
        requireStatus(PurchaseStatus::Sent, "приёмка");
        status_ = PurchaseStatus::Received;
    }
    void cancel() {
        if (status_ == PurchaseStatus::Received) {
            throw InvalidOrderStateException("заказ поставщику уже получен");
        }
        status_ = PurchaseStatus::Cancelled;
    }

private:
    void requireStatus(PurchaseStatus expected, const std::string& action) const {
        if (status_ != expected) {
            throw InvalidOrderStateException("заказ поставщику: " + action);
        }
    }

    int id_;
    Supplier supplier_;
    std::vector<OrderLine> lines_;
    Date orderedOn_;
    PurchaseStatus status_ = PurchaseStatus::Draft;
};

/// Инструмент, требующий периодической калибровки.
class Tool {
public:
    Tool(std::string name, std::string serialNumber, const Date& lastCalibration, int intervalDays)
        : name_(std::move(name)), serialNumber_(std::move(serialNumber)), lastCalibration_(lastCalibration),
          intervalDays_(intervalDays) {}

    const std::string& getName() const { return name_; }
    const std::string& getSerialNumber() const { return serialNumber_; }
    const Date& getLastCalibration() const { return lastCalibration_; }
    int getIntervalDays() const { return intervalDays_; }
    Date nextCalibrationDate() const { return lastCalibration_.plusDays(intervalDays_); }
    bool needsCalibrationOn(const Date& date) const { return !date.isBefore(nextCalibrationDate()); }
    void calibrate(const Date& date) { lastCalibration_ = date; }

private:
    std::string name_;
    std::string serialNumber_;
    Date lastCalibration_;
    int intervalDays_;
};

}  // namespace autoshop
