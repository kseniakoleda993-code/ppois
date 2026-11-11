#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "People.h"
#include "Parts.h"
#include "Services.h"
#include "Vehicle.h"

namespace autoshop {

enum class AppointmentStatus { Scheduled, Confirmed, Cancelled, Completed };

/// Запись клиента на обслуживание.
class Appointment {
public:
    Appointment(int id, const Client& client, const Vehicle& vehicle, const TimeSlot& slot, const ServiceType& service)
        : id_(id), clientId_(client.getId()), vin_(vehicle.getVin()), slot_(slot), service_(service) {
        if (!client.ownsVehicle(vehicle)) {
            throw InvalidVehicleDataException("автомобиль не принадлежит клиенту");
        }
    }

    int getId() const { return id_; }
    int getClientId() const { return clientId_; }
    const std::string& getVin() const { return vin_; }
    const TimeSlot& getSlot() const { return slot_; }
    const ServiceType& getService() const { return service_; }
    AppointmentStatus getStatus() const { return status_; }
    bool isActive() const {
        return status_ == AppointmentStatus::Scheduled || status_ == AppointmentStatus::Confirmed;
    }
    void confirm() {
        if (status_ != AppointmentStatus::Scheduled) {
            throw InvalidOrderStateException("подтвердить можно только новую запись");
        }
        status_ = AppointmentStatus::Confirmed;
    }
    void cancel() {
        if (status_ == AppointmentStatus::Completed) {
            throw InvalidOrderStateException("запись уже выполнена");
        }
        status_ = AppointmentStatus::Cancelled;
    }
    void reschedule(const TimeSlot& newSlot) {
        if (!isActive()) {
            throw InvalidOrderStateException("нельзя перенести неактивную запись");
        }
        slot_ = newSlot;
        status_ = AppointmentStatus::Scheduled;
    }
    void complete() {
        if (status_ != AppointmentStatus::Confirmed) {
            throw InvalidOrderStateException("выполнить можно только подтверждённую запись");
        }
        status_ = AppointmentStatus::Completed;
    }
    bool conflictsWith(const Appointment& other) const {
        return isActive() && other.isActive() && slot_.overlaps(other.slot_);
    }

private:
    int id_;
    int clientId_;
    std::string vin_;
    TimeSlot slot_;
    ServiceType service_;
    AppointmentStatus status_ = AppointmentStatus::Scheduled;
};

/// Журнал записей: следит, чтобы не было больше записей, чем постов.
class AppointmentBook {
public:
    explicit AppointmentBook(int capacity) : capacity_(capacity) {}

    int getCapacity() const { return capacity_; }
    const std::vector<Appointment>& getAppointments() const { return appointments_; }
    int countActiveAt(const TimeSlot& slot) const {
        return static_cast<int>(std::count_if(appointments_.begin(), appointments_.end(), [&](const Appointment& a) {
            return a.isActive() && a.getSlot().overlaps(slot);
        }));
    }
    bool isSlotFree(const TimeSlot& slot) const { return countActiveAt(slot) < capacity_; }
    void book(const Appointment& appointment) {
        if (!isSlotFree(appointment.getSlot())) {
            throw AppointmentConflictException(appointment.getSlot().getDate().toString());
        }
        appointments_.push_back(appointment);
    }
    const Appointment& findById(int id) const {
        for (const Appointment& appointment : appointments_) {
            if (appointment.getId() == id) {
                return appointment;
            }
        }
        throw NotFoundException("запись " + std::to_string(id));
    }
    void cancel(int id) { locate(id).cancel(); }
    void confirm(int id) { locate(id).confirm(); }
    std::vector<Appointment> appointmentsOn(const Date& date) const {
        std::vector<Appointment> result;
        std::copy_if(appointments_.begin(), appointments_.end(), std::back_inserter(result),
                     [&](const Appointment& a) { return a.getSlot().isOn(date); });
        return result;
    }
    std::optional<TimeSlot> findFreeSlot(const Date& date, int durationHours) const {
        for (int start = kWorkDayStartHour; start + durationHours <= kWorkDayEndHour; ++start) {
            const TimeSlot candidate(date, start, durationHours);
            if (isSlotFree(candidate)) {
                return candidate;
            }
        }
        return std::nullopt;
    }

private:
    Appointment& locate(int id) {
        for (Appointment& appointment : appointments_) {
            if (appointment.getId() == id) {
                return appointment;
            }
        }
        throw NotFoundException("запись " + std::to_string(id));
    }

    int capacity_;
    std::vector<Appointment> appointments_;
};

/// Предварительная смета на ремонт.
class Estimate {
public:
    Estimate(int id, const Client& client, const Date& validUntil, double markupPercent)
        : id_(id), clientId_(client.getId()), validUntil_(validUntil), markupPercent_(markupPercent) {}

    int getId() const { return id_; }
    int getClientId() const { return clientId_; }
    const std::vector<ServiceType>& getServices() const { return services_; }
    const std::vector<OrderLine>& getParts() const { return parts_; }
    const Date& getValidUntil() const { return validUntil_; }
    double getMarkupPercent() const { return markupPercent_; }
    bool isApproved() const { return approved_; }
    void addService(const ServiceType& service) { services_.push_back(service); }
    void addPart(const OrderLine& line) { parts_.push_back(line); }
    Money subtotal(const Money& hourlyRate) const {
        Money sum;
        for (const ServiceType& service : services_) {
            sum = sum.plus(service.totalPrice(hourlyRate));
        }
        for (const OrderLine& line : parts_) {
            sum = sum.plus(line.lineTotal());
        }
        return sum;
    }
    Money total(const Money& hourlyRate) const {
        const Money base = subtotal(hourlyRate);
        return base.plus(base.applyPercent(markupPercent_));
    }
    bool isExpiredOn(const Date& date) const { return validUntil_.isBefore(date); }
    void approve(const Date& date) {
        if (isExpiredOn(date)) {
            throw InvalidOrderStateException("смета просрочена");
        }
        approved_ = true;
    }

private:
    int id_;
    int clientId_;
    std::vector<ServiceType> services_;
    std::vector<OrderLine> parts_;
    Date validUntil_;
    double markupPercent_;
    bool approved_ = false;
};

enum class WorkOrderStatus { Opened, InProgress, Completed, Cancelled };

/// Заказ-наряд: основной документ ремонта.
class WorkOrder {
public:
    WorkOrder(int id, const Client& client, const Vehicle& vehicle, const Date& openedOn)
        : id_(id), clientId_(client.getId()), vin_(vehicle.getVin()), openedOn_(openedOn) {
        if (!client.ownsVehicle(vehicle)) {
            throw InvalidOrderStateException("автомобиль не принадлежит клиенту");
        }
    }

    int getId() const { return id_; }
    int getClientId() const { return clientId_; }
    const std::string& getVin() const { return vin_; }
    int getMechanicId() const { return mechanicId_; }
    const std::vector<ServiceType>& getServices() const { return services_; }
    const std::vector<OrderLine>& getParts() const { return parts_; }
    WorkOrderStatus getStatus() const { return status_; }
    const Date& getOpenedOn() const { return openedOn_; }
    double getLaborHours() const { return laborHours_; }
    double getDiscountPercent() const { return discountPercent_; }
    void assignMechanic(const Mechanic& mechanic) {
        requireStatus(WorkOrderStatus::Opened, "назначение механика");
        mechanicId_ = mechanic.getEmployeeId();
    }
    void addService(const ServiceType& service) {
        requireEditable("добавление услуги");
        services_.push_back(service);
    }
    void addPart(const OrderLine& line) {
        requireEditable("добавление запчасти");
        parts_.push_back(line);
    }
    void applyDiscount(const Discount& discount, const Date& date) {
        if (!discount.isValidOn(date)) {
            throw InvalidDateException("скидка не действует на дату " + date.toString());
        }
        discountPercent_ = discount.getPercent();
    }
    void start() {
        requireStatus(WorkOrderStatus::Opened, "начало работ");
        if (mechanicId_ == 0) {
            throw InvalidOrderStateException("не назначен механик");
        }
        status_ = WorkOrderStatus::InProgress;
    }
    void complete(double actualHours) {
        requireStatus(WorkOrderStatus::InProgress, "завершение работ");
        if (actualHours <= 0.0) {
            throw InvalidOrderStateException("фактическое время должно быть положительным");
        }
        laborHours_ = actualHours;
        status_ = WorkOrderStatus::Completed;
    }
    void cancel() {
        if (status_ == WorkOrderStatus::Completed) {
            throw InvalidOrderStateException("завершённый заказ нельзя отменить");
        }
        status_ = WorkOrderStatus::Cancelled;
    }
    double plannedHours() const {
        double hours = 0.0;
        for (const ServiceType& service : services_) {
            hours += service.getStandardHours();
        }
        return hours;
    }
    Money servicesTotal() const {
        Money sum;
        for (const ServiceType& service : services_) {
            sum = sum.plus(service.getBasePrice());
        }
        return sum;
    }
    Money partsTotal() const {
        Money sum;
        for (const OrderLine& line : parts_) {
            sum = sum.plus(line.lineTotal());
        }
        return sum;
    }
    Money laborTotal(const Money& hourlyRate) const {
        return hourlyRate.multiply(laborHours_ > 0.0 ? laborHours_ : plannedHours());
    }
    Money total(const Money& hourlyRate) const {
        const Money subtotal = servicesTotal().plus(laborTotal(hourlyRate)).plus(partsTotal());
        return subtotal.minus(subtotal.applyPercent(discountPercent_));
    }

private:
    void requireStatus(WorkOrderStatus expected, const std::string& action) const {
        if (status_ != expected) {
            throw InvalidOrderStateException(action);
        }
    }
    void requireEditable(const std::string& action) const {
        if (status_ != WorkOrderStatus::Opened && status_ != WorkOrderStatus::InProgress) {
            throw InvalidOrderStateException(action);
        }
    }

    int id_;
    int clientId_;
    std::string vin_;
    int mechanicId_ = 0;
    std::vector<ServiceType> services_;
    std::vector<OrderLine> parts_;
    WorkOrderStatus status_ = WorkOrderStatus::Opened;
    Date openedOn_;
    double laborHours_ = 0.0;
    double discountPercent_ = 0.0;
};

/// Рабочий пост (подъёмник, яма) с набором инструментов.
class WorkshopBay {
public:
    WorkshopBay(int number, std::string type) : number_(number), type_(std::move(type)) {}

    int getNumber() const { return number_; }
    const std::string& getType() const { return type_; }
    int getOccupiedByOrderId() const { return occupiedByOrderId_; }
    const std::vector<Tool>& getTools() const { return tools_; }
    bool isFree() const { return occupiedByOrderId_ == 0; }
    void occupy(const WorkOrder& order) {
        if (!isFree()) {
            throw BayNotAvailableException("пост " + std::to_string(number_) + " занят");
        }
        occupiedByOrderId_ = order.getId();
    }
    void release() { occupiedByOrderId_ = 0; }
    void addTool(const Tool& tool) { tools_.push_back(tool); }
    bool hasTool(const std::string& name) const {
        return std::any_of(tools_.begin(), tools_.end(), [&](const Tool& tool) { return tool.getName() == name; });
    }
    std::vector<std::string> toolsNeedingCalibration(const Date& date) const {
        std::vector<std::string> names;
        for (const Tool& tool : tools_) {
            if (tool.needsCalibrationOn(date)) {
                names.push_back(tool.getName());
            }
        }
        return names;
    }

private:
    int number_;
    std::string type_;
    int occupiedByOrderId_ = 0;
    std::vector<Tool> tools_;
};

/// Контроль качества выполненных работ.
class QualityCheck {
public:
    QualityCheck(const WorkOrder& order, const Mechanic& inspector, const Date& checkedOn)
        : workOrderId_(order.getId()), inspectorId_(inspector.getEmployeeId()), checkedOn_(checkedOn) {}

    int getWorkOrderId() const { return workOrderId_; }
    int getInspectorId() const { return inspectorId_; }
    const Date& getCheckedOn() const { return checkedOn_; }
    const std::vector<std::pair<std::string, bool>>& getItems() const { return items_; }
    void recordItem(const std::string& name, bool passed) { items_.emplace_back(name, passed); }
    bool hasPassed() const {
        return !items_.empty() && std::all_of(items_.begin(), items_.end(), [](const auto& item) { return item.second; });
    }
    std::vector<std::string> failedItems() const {
        std::vector<std::string> names;
        for (const auto& item : items_) {
            if (!item.second) {
                names.push_back(item.first);
            }
        }
        return names;
    }
    double passRatePercent() const {
        if (items_.empty()) {
            return 0.0;
        }
        return kPercentBase * static_cast<double>(items_.size() - failedItems().size()) / static_cast<double>(items_.size());
    }

private:
    int workOrderId_;
    int inspectorId_;
    Date checkedOn_;
    std::vector<std::pair<std::string, bool>> items_;
};

/// Отчёт о диагностике автомобиля.
class InspectionReport {
public:
    InspectionReport(const Vehicle& vehicle, const Mechanic& mechanic, const Date& date)
        : vin_(vehicle.getVin()), mechanicId_(mechanic.getEmployeeId()), date_(date) {}

    const std::string& getVin() const { return vin_; }
    int getMechanicId() const { return mechanicId_; }
    const Date& getDate() const { return date_; }
    const std::vector<DiagnosticResult>& getResults() const { return results_; }
    void addResult(const DiagnosticResult& result) { results_.push_back(result); }
    Money totalEstimatedCost() const {
        Money sum;
        for (const DiagnosticResult& result : results_) {
            sum = sum.plus(result.getEstimatedCost());
        }
        return sum;
    }
    bool hasCriticalIssues() const {
        return std::any_of(results_.begin(), results_.end(), [](const DiagnosticResult& r) { return r.isCritical(); });
    }
    int worstSeverity() const {
        int worst = 0;
        for (const DiagnosticResult& result : results_) {
            worst = std::max(worst, result.getSeverity());
        }
        return worst;
    }

private:
    std::string vin_;
    int mechanicId_;
    Date date_;
    std::vector<DiagnosticResult> results_;
};

enum class ClaimStatus { Pending, Approved, Rejected };

/// Гарантийное обращение по выполненному заказу.
class WarrantyClaim {
public:
    WarrantyClaim(int id, const WorkOrder& order, const Warranty& warranty, std::string reason)
        : id_(id), workOrderId_(order.getId()), warranty_(warranty), reason_(std::move(reason)) {}

    int getId() const { return id_; }
    int getWorkOrderId() const { return workOrderId_; }
    const Warranty& getWarranty() const { return warranty_; }
    const std::string& getReason() const { return reason_; }
    ClaimStatus getStatus() const { return status_; }
    void approve(const Vehicle& vehicle, const Date& date) {
        if (!vehicle.isUnderWarranty(warranty_, date)) {
            status_ = ClaimStatus::Rejected;
            throw WarrantyExpiredException(vehicle.getVin());
        }
        status_ = ClaimStatus::Approved;
    }
    void reject() { status_ = ClaimStatus::Rejected; }
    bool isResolved() const { return status_ != ClaimStatus::Pending; }

private:
    int id_;
    int workOrderId_;
    Warranty warranty_;
    std::string reason_;
    ClaimStatus status_ = ClaimStatus::Pending;
};

enum class ComplaintStatus { Open, Escalated, Resolved };

inline constexpr int kComplaintDeadlineDays = 14;

/// Жалоба клиента.
class Complaint {
public:
    Complaint(int id, const Client& client, const WorkOrder& order, std::string text, const Date& openedOn)
        : id_(id), clientId_(client.getId()), workOrderId_(order.getId()), text_(std::move(text)),
          openedOn_(openedOn) {}

    int getId() const { return id_; }
    int getClientId() const { return clientId_; }
    int getWorkOrderId() const { return workOrderId_; }
    const std::string& getText() const { return text_; }
    ComplaintStatus getStatus() const { return status_; }
    const Date& getOpenedOn() const { return openedOn_; }
    int getResolvedById() const { return resolvedById_; }
    void escalate() {
        if (status_ == ComplaintStatus::Resolved) {
            throw InvalidOrderStateException("жалоба уже закрыта");
        }
        status_ = ComplaintStatus::Escalated;
    }
    void resolve(const Manager& manager, const std::string& note) {
        resolvedById_ = manager.getEmployeeId();
        resolution_ = note;
        status_ = ComplaintStatus::Resolved;
    }
    int ageInDaysOn(const Date& date) const { return openedOn_.daysUntil(date); }
    bool isOverdueOn(const Date& date) const {
        return status_ != ComplaintStatus::Resolved && ageInDaysOn(date) > kComplaintDeadlineDays;
    }

private:
    int id_;
    int clientId_;
    int workOrderId_;
    std::string text_;
    ComplaintStatus status_ = ComplaintStatus::Open;
    Date openedOn_;
    int resolvedById_ = 0;
    std::string resolution_;
};

inline constexpr int kMinRating = 1;
inline constexpr int kMaxRating = 5;
inline constexpr int kPositiveRating = 4;

/// Отзыв клиента о выполненном заказе.
class CustomerReview {
public:
    CustomerReview(const Client& client, const WorkOrder& order, int rating, std::string comment)
        : clientId_(client.getId()), workOrderId_(order.getId()), rating_(rating), comment_(std::move(comment)) {
        if (rating_ < kMinRating || rating_ > kMaxRating) {
            throw InvalidClientDataException("оценка отзыва вне диапазона");
        }
    }

    int getClientId() const { return clientId_; }
    int getWorkOrderId() const { return workOrderId_; }
    int getRating() const { return rating_; }
    const std::string& getComment() const { return comment_; }
    bool isPositive() const { return rating_ >= kPositiveRating; }
    std::string summary() const { return std::to_string(rating_) + "/" + std::to_string(kMaxRating) + ": " + comment_; }

private:
    int clientId_;
    int workOrderId_;
    int rating_;
    std::string comment_;
};

}  // namespace autoshop
