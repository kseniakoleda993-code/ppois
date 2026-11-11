#pragma once

#include <string>
#include <utility>

#include "Common.h"

namespace autoshop {

inline constexpr double kLongServiceHours = 8.0;
inline constexpr double kMaxDiscountPercent = 50.0;

/// Вид услуги, которую оказывает автомастерская (замена масла, диагностика и т. д.).
class ServiceType {
public:
    ServiceType(std::string code, std::string name, const Money& basePrice, double standardHours,
                std::string requiredSkill)
        : code_(std::move(code)), name_(std::move(name)), basePrice_(basePrice),
          standardHours_(standardHours), requiredSkill_(std::move(requiredSkill)) {
        if (code_.empty() || name_.empty() || standardHours_ <= 0.0 || basePrice_.isNegative()) {
            throw InvalidServiceDataException(name_);
        }
    }

    const std::string& getCode() const { return code_; }
    const std::string& getName() const { return name_; }
    const Money& getBasePrice() const { return basePrice_; }
    double getStandardHours() const { return standardHours_; }
    const std::string& getRequiredSkill() const { return requiredSkill_; }
    Money totalPrice(const Money& hourlyRate) const { return basePrice_.plus(hourlyRate.multiply(standardHours_)); }
    bool isLongRunning() const { return standardHours_ >= kLongServiceHours; }
    bool requiresSkill(const std::string& skill) const { return requiredSkill_ == skill; }

private:
    std::string code_;
    std::string name_;
    Money basePrice_;
    double standardHours_;
    std::string requiredSkill_;
};

/// Скидка с ограниченным сроком действия.
class Discount {
public:
    Discount(double percent, std::string reason, const Date& validFrom, const Date& validTo)
        : percent_(percent), reason_(std::move(reason)), validFrom_(validFrom), validTo_(validTo) {
        if (percent_ <= 0.0 || percent_ > kMaxDiscountPercent) {
            throw DiscountLimitExceededException(std::to_string(percent_) + "%");
        }
        if (validTo_.isBefore(validFrom_)) {
            throw InvalidDateException("конец действия скидки раньше начала");
        }
    }

    double getPercent() const { return percent_; }
    const std::string& getReason() const { return reason_; }
    const Date& getValidFrom() const { return validFrom_; }
    const Date& getValidTo() const { return validTo_; }
    bool isValidOn(const Date& date) const { return !date.isBefore(validFrom_) && !validTo_.isBefore(date); }
    Money savedAmount(const Money& price) const { return price.applyPercent(percent_); }
    Money applyTo(const Money& price) const { return price.minus(savedAmount(price)); }
    void extendTo(const Date& newEnd) {
        if (newEnd.isBefore(validTo_)) {
            throw InvalidDateException("новый срок раньше текущего");
        }
        validTo_ = newEnd;
    }

private:
    double percent_;
    std::string reason_;
    Date validFrom_;
    Date validTo_;
};

/// Промокод, дающий скидку ограниченное число раз.
class PromoCode {
public:
    PromoCode(std::string code, const Discount& discount, int usageLimit)
        : code_(std::move(code)), discount_(discount), usageLimit_(usageLimit) {}

    const std::string& getCode() const { return code_; }
    const Discount& getDiscount() const { return discount_; }
    int getTimesUsed() const { return timesUsed_; }
    int remainingUses() const { return usageLimit_ - timesUsed_; }
    bool canBeUsedOn(const Date& date) const { return remainingUses() > 0 && discount_.isValidOn(date); }
    Discount redeem(const Date& date) {
        if (!canBeUsedOn(date)) {
            throw PromoCodeException(code_);
        }
        ++timesUsed_;
        return discount_;
    }

private:
    std::string code_;
    Discount discount_;
    int usageLimit_;
    int timesUsed_ = 0;
};

}  // namespace autoshop
