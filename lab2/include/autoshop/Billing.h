#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Operations.h"
#include "People.h"
#include "Services.h"

namespace autoshop {

inline constexpr int kPaymentTermDays = 10;

/// Счёт на оплату выполненного заказа.
class Invoice {
public:
    Invoice(int id, const WorkOrder& order, const Money& amount, const Date& issuedOn)
        : id_(id), workOrderId_(order.getId()), clientId_(order.getClientId()), issuedOn_(issuedOn),
          dueOn_(issuedOn.plusDays(kPaymentTermDays)), amount_(amount) {
        if (amount_.isNegative()) {
            throw InvalidMoneyOperationException("отрицательная сумма счёта");
        }
        if (order.getStatus() != WorkOrderStatus::Completed) {
            throw InvalidOrderStateException("счёт выставляется после завершения работ");
        }
    }

    int getId() const { return id_; }
    int getWorkOrderId() const { return workOrderId_; }
    int getClientId() const { return clientId_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    const Date& getDueOn() const { return dueOn_; }
    const Money& getAmount() const { return amount_; }
    const Money& getPaid() const { return paid_; }
    Money outstanding() const { return amount_.minus(paid_); }
    bool isPaid() const { return outstanding().isZero(); }
    void applyDiscount(const Discount& discount, const Date& date) {
        if (!discount.isValidOn(date)) {
            throw InvalidDateException("скидка не действует на дату " + date.toString());
        }
        if (!paid_.isZero()) {
            throw InvalidOrderStateException("скидка после частичной оплаты недопустима");
        }
        amount_ = discount.applyTo(amount_);
    }
    void registerPayment(const Money& payment) {
        if (!outstanding().isGreaterOrEqual(payment)) {
            throw InvalidMoneyOperationException("сумма платежа больше остатка по счёту");
        }
        paid_ = paid_.plus(payment);
    }
    bool isOverdueOn(const Date& date) const { return !isPaid() && dueOn_.isBefore(date); }
    int daysOverdue(const Date& date) const { return isOverdueOn(date) ? dueOn_.daysUntil(date) : 0; }

private:
    int id_;
    int workOrderId_;
    int clientId_;
    Date issuedOn_;
    Date dueOn_;
    Money amount_;
    Money paid_;
};

inline constexpr const char* kCashMethod = "cash";
inline constexpr const char* kCardMethod = "card";
inline constexpr const char* kTransferMethod = "transfer";
inline constexpr double kCardFeePercent = 1.5;

/// Платёж по счёту.
class Payment {
public:
    Payment(int id, const Invoice& invoice, const Money& amount, std::string method, const Date& paidOn)
        : id_(id), invoiceId_(invoice.getId()), amount_(amount), method_(std::move(method)), paidOn_(paidOn) {
        if (amount_.isNegative() || amount_.isZero() || !invoice.outstanding().isGreaterOrEqual(amount_)) {
            throw InvalidMoneyOperationException("некорректная сумма платежа");
        }
        if (method_ != kCashMethod && method_ != kCardMethod && method_ != kTransferMethod) {
            throw InvalidMoneyOperationException("неизвестный способ оплаты " + method_);
        }
    }

    int getId() const { return id_; }
    int getInvoiceId() const { return invoiceId_; }
    const Money& getAmount() const { return amount_; }
    const std::string& getMethod() const { return method_; }
    const Date& getPaidOn() const { return paidOn_; }
    bool isCash() const { return method_ == kCashMethod; }
    Money processingFee() const { return method_ == kCardMethod ? amount_.applyPercent(kCardFeePercent) : Money(); }
    Money netAmount() const { return amount_.minus(processingFee()); }

private:
    int id_;
    int invoiceId_;
    Money amount_;
    std::string method_;
    Date paidOn_;
};

/// Кассовый чек.
class Receipt {
public:
    Receipt(int number, const Payment& payment, const Cashier& cashier)
        : number_(number), paymentId_(payment.getId()), total_(payment.getAmount()), cashierName_(cashier.getName()),
          issuedOn_(payment.getPaidOn()) {}

    int getNumber() const { return number_; }
    int getPaymentId() const { return paymentId_; }
    const Money& getTotal() const { return total_; }
    const std::string& getCashierName() const { return cashierName_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    std::string render() const {
        return "Чек №" + std::to_string(number_) + " от " + issuedOn_.toString() + ", кассир " + cashierName_ +
               ", итого " + total_.toString();
    }

private:
    int number_;
    int paymentId_;
    Money total_;
    std::string cashierName_;
    Date issuedOn_;
};

/// Возврат денег клиенту.
class Refund {
public:
    Refund(int id, const Payment& payment, const Money& amount, std::string reason)
        : id_(id), paymentId_(payment.getId()), amount_(amount), reason_(std::move(reason)) {
        if (!payment.getAmount().isGreaterOrEqual(amount_)) {
            throw InsufficientFundsException("возврат больше суммы платежа");
        }
    }

    int getId() const { return id_; }
    int getPaymentId() const { return paymentId_; }
    const Money& getAmount() const { return amount_; }
    const std::string& getReason() const { return reason_; }
    bool isFullRefundOf(const Payment& payment) const { return amount_ == payment.getAmount(); }
    double refundedSharePercent(const Payment& payment) const {
        return kPercentBase * static_cast<double>(amount_.getCents()) / static_cast<double>(payment.getAmount().getCents());
    }

private:
    int id_;
    int paymentId_;
    Money amount_;
    std::string reason_;
};

/// Расчётный листок сотрудника за период.
class PayrollRecord {
public:
    PayrollRecord(const Employee& employee, std::string period, const Money& bonus, double taxPercent)
        : employeeId_(employee.getEmployeeId()), period_(std::move(period)), bonus_(bonus),
          gross_(employee.getMonthlySalary().plus(bonus)), tax_(gross_.applyPercent(taxPercent)) {}

    int getEmployeeId() const { return employeeId_; }
    const std::string& getPeriod() const { return period_; }
    const Money& getBonus() const { return bonus_; }
    const Money& getGross() const { return gross_; }
    const Money& getTax() const { return tax_; }
    Money net() const { return gross_.minus(tax_); }
    double effectiveTaxPercent() const {
        return gross_.isZero() ? 0.0 : kPercentBase * static_cast<double>(tax_.getCents()) / static_cast<double>(gross_.getCents());
    }

private:
    int employeeId_;
    std::string period_;
    Money bonus_;
    Money gross_;
    Money tax_;
};

inline const Money kLargeExpenseThreshold = Money::fromUnits(500);

/// Расход мастерской.
class ExpenseRecord {
public:
    ExpenseRecord(std::string category, const Money& amount, const Date& date, std::string description)
        : category_(std::move(category)), amount_(amount), date_(date), description_(std::move(description)) {}

    const std::string& getCategory() const { return category_; }
    const Money& getAmount() const { return amount_; }
    const Date& getDate() const { return date_; }
    const std::string& getDescription() const { return description_; }
    bool isInMonth(int year, int month) const { return date_.getYear() == year && date_.getMonth() == month; }
    bool isLarge() const { return amount_.isGreaterOrEqual(kLargeExpenseThreshold); }

private:
    std::string category_;
    Money amount_;
    Date date_;
    std::string description_;
};

/// Финансовый отчёт за период.
class FinancialReport {
public:
    FinancialReport(const Date& periodStart, const Date& periodEnd) : periodStart_(periodStart), periodEnd_(periodEnd) {
        if (periodEnd_.isBefore(periodStart_)) {
            throw InvalidDateException("конец периода раньше начала");
        }
    }

    const Date& getPeriodStart() const { return periodStart_; }
    const Date& getPeriodEnd() const { return periodEnd_; }
    const Money& getRevenue() const { return revenue_; }
    const Money& getExpenses() const { return expenses_; }
    bool includes(const Date& date) const { return !date.isBefore(periodStart_) && !periodEnd_.isBefore(date); }
    void addInvoice(const Invoice& invoice) {
        if (includes(invoice.getIssuedOn())) {
            revenue_ = revenue_.plus(invoice.getPaid());
        }
    }
    void addExpense(const ExpenseRecord& expense) {
        if (includes(expense.getDate())) {
            expenses_ = expenses_.plus(expense.getAmount());
        }
    }
    Money profit() const { return revenue_.minus(expenses_); }
    bool isProfitable() const { return !profit().isNegative() && !profit().isZero(); }
    double profitMarginPercent() const {
        return revenue_.isZero() ? 0.0 : kPercentBase * static_cast<double>(profit().getCents()) / static_cast<double>(revenue_.getCents());
    }

private:
    Date periodStart_;
    Date periodEnd_;
    Money revenue_;
    Money expenses_;
};

/// Бухгалтер: считает зарплату и составляет отчёты.
class Accountant : public Employee {
public:
    Accountant(const Employee& employee, double taxPercent) : Employee(employee), taxPercent_(taxPercent) {}

    double getTaxPercent() const { return taxPercent_; }
    int getReportsPrepared() const { return reportsPrepared_; }
    PayrollRecord calculatePayroll(const Employee& employee, const std::string& period, const Money& bonus) const {
        return PayrollRecord(employee, period, bonus, taxPercent_);
    }
    FinancialReport compileReport(const std::vector<Invoice>& invoices, const std::vector<ExpenseRecord>& expenses,
                                  const Date& start, const Date& end) {
        FinancialReport report(start, end);
        for (const Invoice& invoice : invoices) {
            report.addInvoice(invoice);
        }
        for (const ExpenseRecord& expense : expenses) {
            report.addExpense(expense);
        }
        ++reportsPrepared_;
        return report;
    }

private:
    double taxPercent_;
    int reportsPrepared_ = 0;
};

inline constexpr int kMaxShiftHours = 12;
inline constexpr int kLastStartHour = 23;
inline constexpr int kNightStartHour = 22;
inline constexpr int kNightEndHour = 6;

/// Рабочая смена сотрудника.
class Shift {
public:
    Shift(const Employee& employee, const Date& date, int startHour, int hours)
        : employeeId_(employee.getEmployeeId()), date_(date), startHour_(startHour), hours_(hours) {
        if (startHour_ < 0 || startHour_ > kLastStartHour || hours_ <= 0 || hours_ > kMaxShiftHours) {
            throw InvalidDateException("некорректная смена");
        }
    }

    int getEmployeeId() const { return employeeId_; }
    const Date& getDate() const { return date_; }
    int getStartHour() const { return startHour_; }
    int getHours() const { return hours_; }
    int endHour() const { return startHour_ + hours_; }
    bool isNightShift() const { return startHour_ >= kNightStartHour || startHour_ < kNightEndHour; }
    bool overlaps(const Shift& other) const {
        return employeeId_ == other.employeeId_ && date_ == other.date_ && startHour_ < other.endHour() &&
               other.startHour_ < endHour();
    }

private:
    int employeeId_;
    Date date_;
    int startHour_;
    int hours_;
};

/// График смен сотрудников.
class WorkSchedule {
public:
    explicit WorkSchedule(int maxTotalHours) : maxTotalHours_(maxTotalHours) {}

    int getMaxTotalHours() const { return maxTotalHours_; }
    const std::vector<Shift>& getShifts() const { return shifts_; }
    int hoursFor(const Employee& employee) const {
        int total = 0;
        for (const Shift& shift : shifts_) {
            if (shift.getEmployeeId() == employee.getEmployeeId()) {
                total += shift.getHours();
            }
        }
        return total;
    }
    void addShift(const Shift& shift) {
        for (const Shift& existing : shifts_) {
            if (existing.overlaps(shift)) {
                throw AppointmentConflictException("пересечение смен сотрудника " + std::to_string(shift.getEmployeeId()));
            }
        }
        shifts_.push_back(shift);
    }
    std::vector<Shift> shiftsOn(const Date& date) const {
        std::vector<Shift> result;
        std::copy_if(shifts_.begin(), shifts_.end(), std::back_inserter(result),
                     [&](const Shift& shift) { return shift.getDate() == date; });
        return result;
    }
    bool isOnDuty(const Employee& employee, const Date& date, int hour) const {
        return std::any_of(shifts_.begin(), shifts_.end(), [&](const Shift& shift) {
            return shift.getEmployeeId() == employee.getEmployeeId() && shift.getDate() == date &&
                   hour >= shift.getStartHour() && hour < shift.endHour();
        });
    }
    bool canAddHours(const Employee& employee, int hours) const { return hoursFor(employee) + hours <= maxTotalHours_; }

private:
    std::vector<Shift> shifts_;
    int maxTotalHours_;
};

inline constexpr int kUnitsPerPoint = 10;
inline constexpr int kSilverPoints = 100;
inline constexpr int kGoldPoints = 500;
inline constexpr double kSilverDiscountPercent = 3.0;
inline constexpr double kGoldDiscountPercent = 5.0;

/// Карта лояльности клиента.
class LoyaltyCard {
public:
    LoyaltyCard(const Client& client, const Date& issuedOn) : clientId_(client.getId()), issuedOn_(issuedOn) {}

    int getClientId() const { return clientId_; }
    int getPoints() const { return points_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    void earn(const Money& spent) { points_ += static_cast<int>(spent.getCents() / kCentsPerUnit / kUnitsPerPoint); }
    void redeem(int points) {
        if (points > points_) {
            throw InsufficientFundsException("недостаточно баллов");
        }
        points_ -= points;
    }
    std::string tier() const {
        if (points_ >= kGoldPoints) {
            return "Gold";
        }
        return points_ >= kSilverPoints ? "Silver" : "Bronze";
    }
    double discountPercent() const {
        if (points_ >= kGoldPoints) {
            return kGoldDiscountPercent;
        }
        return points_ >= kSilverPoints ? kSilverDiscountPercent : 0.0;
    }

private:
    int clientId_;
    int points_ = 0;
    Date issuedOn_;
};

inline constexpr const char* kEmailChannel = "email";
inline constexpr const char* kSmsChannel = "sms";

/// Уведомление клиенту.
class Notification {
public:
    Notification(int recipientId, std::string message, std::string channel, const Date& createdOn)
        : recipientId_(recipientId), message_(std::move(message)), channel_(std::move(channel)), createdOn_(createdOn) {}

    int getRecipientId() const { return recipientId_; }
    const std::string& getMessage() const { return message_; }
    const std::string& getChannel() const { return channel_; }
    const Date& getCreatedOn() const { return createdOn_; }
    bool isEmail() const { return channel_ == kEmailChannel; }

private:
    int recipientId_;
    std::string message_;
    std::string channel_;
    Date createdOn_;
};

/// Служба уведомлений (сообщения накапливаются в журнале отправленных).
class NotificationService {
public:
    NotificationService() = default;

    const std::vector<Notification>& getSent() const { return sent_; }
    void notifyAppointment(const Client& client, const Appointment& appointment) {
        send(client, "Запись №" + std::to_string(appointment.getId()) + " на " + appointment.getSlot().getDate().toString(),
             appointment.getSlot().getDate());
    }
    void notifyInvoice(const Client& client, const Invoice& invoice) {
        send(client, "Счёт №" + std::to_string(invoice.getId()) + " на сумму " + invoice.getAmount().toString(),
             invoice.getIssuedOn());
    }
    void notifyWorkReady(const Client& client, const WorkOrder& order, const Date& date) {
        send(client, "Заказ-наряд №" + std::to_string(order.getId()) + " выполнен", date);
    }
    int countFor(const Client& client) const {
        return static_cast<int>(std::count_if(sent_.begin(), sent_.end(), [&](const Notification& n) {
            return n.getRecipientId() == client.getId();
        }));
    }

private:
    void send(const Client& client, const std::string& text, const Date& date) {
        const std::string channel = client.getContact().hasValidEmail() ? kEmailChannel : kSmsChannel;
        sent_.emplace_back(client.getId(), text, channel, date);
    }

    std::vector<Notification> sent_;
};

}  // namespace autoshop
