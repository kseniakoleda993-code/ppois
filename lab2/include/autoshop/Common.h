#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Exceptions.h"

namespace autoshop {

inline constexpr long long kCentsPerUnit = 100;
inline constexpr double kPercentBase = 100.0;
inline constexpr const char* kDefaultCurrency = "BYN";

/// Денежная сумма, хранится в копейках, чтобы избежать ошибок округления.
class Money {
public:
    explicit Money(long long cents = 0, std::string currency = kDefaultCurrency)
        : cents_(cents), currency_(std::move(currency)) {}

    static Money fromUnits(long long units, const std::string& currency = kDefaultCurrency) {
        return Money(units * kCentsPerUnit, currency);
    }
    long long getCents() const { return cents_; }
    const std::string& getCurrency() const { return currency_; }
    Money plus(const Money& other) const {
        requireSameCurrency(other);
        return Money(cents_ + other.cents_, currency_);
    }
    Money minus(const Money& other) const {
        requireSameCurrency(other);
        return Money(cents_ - other.cents_, currency_);
    }
    Money multiply(double factor) const { return Money(std::llround(cents_ * factor), currency_); }
    Money applyPercent(double percent) const { return multiply(percent / kPercentBase); }
    bool isNegative() const { return cents_ < 0; }
    bool isZero() const { return cents_ == 0; }
    bool isGreaterOrEqual(const Money& other) const {
        requireSameCurrency(other);
        return cents_ >= other.cents_;
    }
    std::string toString() const {
        std::ostringstream out;
        out << (cents_ < 0 ? "-" : "") << std::llabs(cents_) / kCentsPerUnit << '.';
        out << std::setw(2) << std::setfill('0') << std::llabs(cents_) % kCentsPerUnit << ' ' << currency_;
        return out.str();
    }
    bool operator==(const Money& other) const { return cents_ == other.cents_ && currency_ == other.currency_; }

private:
    void requireSameCurrency(const Money& other) const {
        if (currency_ != other.currency_) {
            throw InvalidMoneyOperationException("разные валюты " + currency_ + " и " + other.currency_);
        }
    }

    long long cents_;
    std::string currency_;
};

inline constexpr int kDefaultYear = 2000;
inline constexpr int kMinYear = 1900;
inline constexpr int kMonthsInYear = 12;
inline constexpr int kFebruary = 2;
inline constexpr int kLeapFebruaryDays = 29;
inline constexpr int kLeapYearCycle = 4;
inline constexpr int kCenturyCycle = 100;
inline constexpr int kGregorianCycle = 400;
inline constexpr int kDaysPerYear = 365;
inline constexpr int kDaysInMonth[kMonthsInYear] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
inline constexpr int kDateFieldWidth = 2;
inline constexpr int kYearFieldWidth = 4;

/// Календарная дата (григорианский календарь).
class Date {
public:
    Date() = default;
    Date(int year, int month, int day) : year_(year), month_(month), day_(day) { validate(); }

    int getYear() const { return year_; }
    int getMonth() const { return month_; }
    int getDay() const { return day_; }
    bool isLeapYear() const {
        return (year_ % kLeapYearCycle == 0 && year_ % kCenturyCycle != 0) || year_ % kGregorianCycle == 0;
    }
    int daysInMonth() const {
        if (month_ == kFebruary && isLeapYear()) {
            return kLeapFebruaryDays;
        }
        return kDaysInMonth[month_ - 1];
    }
    Date plusDays(int days) const {
        if (days < 0) {
            throw InvalidDateException("отрицательное смещение");
        }
        Date result = *this;
        for (int step = 0; step < days; ++step) {
            result = result.nextDay();
        }
        return result;
    }
    int daysUntil(const Date& other) const {
        if (other.isBefore(*this)) {
            return -other.daysUntil(*this);
        }
        int days = 0;
        for (Date cursor = *this; cursor.isBefore(other); cursor = cursor.nextDay()) {
            ++days;
        }
        return days;
    }
    bool isBefore(const Date& other) const {
        return std::tie(year_, month_, day_) < std::tie(other.year_, other.month_, other.day_);
    }
    std::string toString() const {
        std::ostringstream out;
        out << std::setfill('0') << std::setw(kYearFieldWidth) << year_ << '-';
        out << std::setw(kDateFieldWidth) << month_ << '-' << std::setw(kDateFieldWidth) << day_;
        return out.str();
    }
    bool operator==(const Date& other) const {
        return year_ == other.year_ && month_ == other.month_ && day_ == other.day_;
    }

private:
    Date nextDay() const {
        if (day_ < daysInMonth()) {
            return Date(year_, month_, day_ + 1);
        }
        if (month_ < kMonthsInYear) {
            return Date(year_, month_ + 1, 1);
        }
        return Date(year_ + 1, 1, 1);
    }
    void validate() const {
        if (year_ < kMinYear || month_ < 1 || month_ > kMonthsInYear || day_ < 1 || day_ > daysInMonth()) {
            throw InvalidDateException(std::to_string(year_) + "-" + std::to_string(month_) + "-" + std::to_string(day_));
        }
    }

    int year_ = kDefaultYear;
    int month_ = 1;
    int day_ = 1;
};

inline constexpr int kWorkDayStartHour = 8;
inline constexpr int kWorkDayEndHour = 20;

/// Временной интервал в пределах одного рабочего дня.
class TimeSlot {
public:
    TimeSlot(const Date& date, int startHour, int durationHours)
        : date_(date), startHour_(startHour), durationHours_(durationHours) {
        if (durationHours_ <= 0 || startHour_ < kWorkDayStartHour || endHour() > kWorkDayEndHour) {
            throw InvalidDateException("интервал вне рабочего времени");
        }
    }

    const Date& getDate() const { return date_; }
    int getStartHour() const { return startHour_; }
    int getDurationHours() const { return durationHours_; }
    int endHour() const { return startHour_ + durationHours_; }
    bool isOn(const Date& date) const { return date_ == date; }
    bool contains(int hour) const { return hour >= startHour_ && hour < endHour(); }
    bool overlaps(const TimeSlot& other) const {
        return date_ == other.date_ && startHour_ < other.endHour() && other.startHour_ < endHour();
    }

private:
    Date date_;
    int startHour_;
    int durationHours_;
};

inline constexpr std::size_t kPostalCodeLength = 6;

/// Почтовый адрес.
class Address {
public:
    Address() = default;
    Address(std::string street, std::string city, std::string postalCode)
        : street_(std::move(street)), city_(std::move(city)), postalCode_(std::move(postalCode)) {}

    const std::string& getStreet() const { return street_; }
    const std::string& getCity() const { return city_; }
    const std::string& getPostalCode() const { return postalCode_; }
    std::string fullAddress() const { return postalCode_ + ", " + city_ + ", " + street_; }
    bool isInCity(const std::string& city) const { return city_ == city; }
    bool hasValidPostalCode() const {
        return postalCode_.size() == kPostalCodeLength &&
               std::all_of(postalCode_.begin(), postalCode_.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
    }

private:
    std::string street_;
    std::string city_;
    std::string postalCode_;
};

inline constexpr std::size_t kMinPhoneDigits = 9;
inline constexpr std::size_t kMaxPhoneDigits = 15;
inline constexpr std::size_t kVisiblePhoneTail = 4;

/// Контактные данные: телефон и электронная почта.
class ContactInfo {
public:
    ContactInfo() = default;
    ContactInfo(std::string phone, std::string email) : phone_(std::move(phone)), email_(std::move(email)) {}

    const std::string& getPhone() const { return phone_; }
    const std::string& getEmail() const { return email_; }
    std::string phoneDigits() const {
        std::string digits;
        std::copy_if(phone_.begin(), phone_.end(), std::back_inserter(digits),
                     [](unsigned char c) { return std::isdigit(c) != 0; });
        return digits;
    }
    bool hasValidPhone() const {
        const std::size_t count = phoneDigits().size();
        return count >= kMinPhoneDigits && count <= kMaxPhoneDigits;
    }
    bool hasValidEmail() const {
        const std::size_t at = email_.find('@');
        if (at == std::string::npos || at == 0) {
            return false;
        }
        const std::size_t dot = email_.find('.', at);
        return dot != std::string::npos && dot > at + 1 && dot + 1 < email_.size();
    }
    std::string maskedPhone() const {
        const std::string digits = phoneDigits();
        if (digits.size() <= kVisiblePhoneTail) {
            return digits;
        }
        return std::string(digits.size() - kVisiblePhoneTail, '*') + digits.substr(digits.size() - kVisiblePhoneTail);
    }

private:
    std::string phone_;
    std::string email_;
};

}  // namespace autoshop
