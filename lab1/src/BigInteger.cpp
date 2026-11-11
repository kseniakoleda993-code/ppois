/**
 * @file BigInteger.cpp
 * @author Коледа Ксения
 * @brief Реализация класса BigInteger.
 */
#include "BigInteger.h"

#include <algorithm>
#include <cstddef>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>

namespace lab1 {

namespace {

using Magnitude = std::vector<std::uint32_t>;

constexpr std::uint32_t kBase = 1000000000U;
constexpr std::size_t kBaseDigits = 9;
constexpr int kLess = -1;
constexpr int kEqual = 0;
constexpr int kGreater = 1;

void trimLeadingZeros(Magnitude& magnitude) {
    while (!magnitude.empty() && magnitude.back() == 0) {
        magnitude.pop_back();
    }
}

int compareMagnitudes(const Magnitude& lhs, const Magnitude& rhs) {
    if (lhs.size() != rhs.size()) {
        return lhs.size() < rhs.size() ? kLess : kGreater;
    }
    for (std::size_t index = lhs.size(); index-- > 0;) {
        if (lhs[index] != rhs[index]) {
            return lhs[index] < rhs[index] ? kLess : kGreater;
        }
    }
    return kEqual;
}

Magnitude addMagnitudes(const Magnitude& lhs, const Magnitude& rhs) {
    Magnitude result;
    std::uint64_t carry = 0;
    const std::size_t size = std::max(lhs.size(), rhs.size());
    for (std::size_t index = 0; index < size || carry != 0; ++index) {
        std::uint64_t sum = carry;
        sum += index < lhs.size() ? lhs[index] : 0;
        sum += index < rhs.size() ? rhs[index] : 0;
        result.push_back(static_cast<std::uint32_t>(sum % kBase));
        carry = sum / kBase;
    }
    return result;
}

/// Вычитание модулей; требует lhs >= rhs.
Magnitude subtractMagnitudes(const Magnitude& lhs, const Magnitude& rhs) {
    Magnitude result(lhs);
    std::int64_t borrow = 0;
    for (std::size_t index = 0; index < result.size(); ++index) {
        std::int64_t difference = static_cast<std::int64_t>(result[index]) - borrow;
        difference -= index < rhs.size() ? rhs[index] : 0;
        borrow = difference < 0 ? 1 : 0;
        result[index] = static_cast<std::uint32_t>(difference + borrow * kBase);
    }
    trimLeadingZeros(result);
    return result;
}

Magnitude multiplyMagnitudes(const Magnitude& lhs, const Magnitude& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return {};
    }
    Magnitude result(lhs.size() + rhs.size(), 0);
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        std::uint64_t carry = 0;
        for (std::size_t j = 0; j < rhs.size() || carry != 0; ++j) {
            std::uint64_t current = result[i + j] + carry;
            current += j < rhs.size() ? static_cast<std::uint64_t>(lhs[i]) * rhs[j] : 0;
            result[i + j] = static_cast<std::uint32_t>(current % kBase);
            carry = current / kBase;
        }
    }
    trimLeadingZeros(result);
    return result;
}

Magnitude multiplyByDigit(const Magnitude& magnitude, std::uint32_t digit) {
    if (digit == 0) {
        return {};
    }
    return multiplyMagnitudes(magnitude, Magnitude{digit});
}

/// Подбирает наибольшую цифру q, для которой divisor * q <= remainder (двоичный поиск).
std::uint32_t findQuotientDigit(const Magnitude& remainder, const Magnitude& divisor) {
    std::uint32_t low = 0;
    std::uint32_t high = kBase - 1;
    while (low < high) {
        const std::uint32_t middle = low + (high - low + 1) / 2;
        if (compareMagnitudes(multiplyByDigit(divisor, middle), remainder) <= kEqual) {
            low = middle;
        } else {
            high = middle - 1;
        }
    }
    return low;
}

/// Деление в столбик; требует непустой divisor. Возвращает частное.
Magnitude divideMagnitudes(const Magnitude& dividend, const Magnitude& divisor) {
    Magnitude quotient(dividend.size(), 0);
    Magnitude remainder;
    for (std::size_t index = dividend.size(); index-- > 0;) {
        remainder.insert(remainder.begin(), dividend[index]);
        trimLeadingZeros(remainder);
        const std::uint32_t digit = findQuotientDigit(remainder, divisor);
        quotient[index] = digit;
        remainder = subtractMagnitudes(remainder, multiplyByDigit(divisor, digit));
    }
    trimLeadingZeros(quotient);
    return quotient;
}

bool hasOnlyDigits(const std::string& text, std::size_t start) {
    if (start >= text.size()) {
        return false;
    }
    return std::all_of(text.begin() + static_cast<std::ptrdiff_t>(start), text.end(),
                       [](char symbol) { return symbol >= '0' && symbol <= '9'; });
}

Magnitude parseDigits(const std::string& text, std::size_t start) {
    Magnitude result;
    for (std::size_t end = text.size(); end > start;) {
        const std::size_t chunkStart = end - start > kBaseDigits ? end - kBaseDigits : start;
        result.push_back(static_cast<std::uint32_t>(std::stoul(text.substr(chunkStart, end - chunkStart))));
        end = chunkStart;
    }
    trimLeadingZeros(result);
    return result;
}

}  // namespace

BigInteger::BigInteger(long long value) : negative_(value < 0) {
    const auto raw = static_cast<unsigned long long>(value);
    unsigned long long magnitude = negative_ ? 0ULL - raw : raw;
    while (magnitude != 0) {
        digits_.push_back(static_cast<std::uint32_t>(magnitude % kBase));
        magnitude /= kBase;
    }
}

BigInteger::BigInteger(const std::string& text) {
    const bool hasSign = !text.empty() && (text[0] == '-' || text[0] == '+');
    const std::size_t start = hasSign ? 1 : 0;
    if (!hasOnlyDigits(text, start)) {
        throw std::invalid_argument("Некорректная запись целого числа: \"" + text + "\"");
    }
    negative_ = hasSign && text[0] == '-';
    digits_ = parseDigits(text, start);
    normalize();
}

BigInteger::operator long long() const {
    const BigInteger maxValue(std::numeric_limits<long long>::max());
    const BigInteger minValue(std::numeric_limits<long long>::min());
    if (*this > maxValue || *this < minValue) {
        throw std::overflow_error("Число не помещается в long long");
    }
    unsigned long long magnitude = 0;
    for (std::size_t index = digits_.size(); index-- > 0;) {
        magnitude = magnitude * kBase + digits_[index];
    }
    if (!negative_) {
        return static_cast<long long>(magnitude);
    }
    return -static_cast<long long>(magnitude - 1) - 1;
}

std::string BigInteger::toString() const {
    if (digits_.empty()) {
        return "0";
    }
    std::string result = negative_ ? "-" : "";
    result += std::to_string(digits_.back());
    for (std::size_t index = digits_.size() - 1; index-- > 0;) {
        const std::string chunk = std::to_string(digits_[index]);
        result += std::string(kBaseDigits - chunk.size(), '0') + chunk;
    }
    return result;
}

bool BigInteger::isNegative() const { return negative_; }
bool BigInteger::isZero() const { return digits_.empty(); }

int BigInteger::compareTo(const BigInteger& other) const {
    if (negative_ != other.negative_) {
        return negative_ ? kLess : kGreater;
    }
    const int magnitudeOrder = compareMagnitudes(digits_, other.digits_);
    return negative_ ? -magnitudeOrder : magnitudeOrder;
}

BigInteger BigInteger::operator-() const {
    BigInteger result(*this);
    result.negative_ = !negative_;
    result.normalize();
    return result;
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
    if (negative_ == other.negative_) {
        digits_ = addMagnitudes(digits_, other.digits_);
        return *this;
    }
    if (compareMagnitudes(digits_, other.digits_) >= kEqual) {
        digits_ = subtractMagnitudes(digits_, other.digits_);
    } else {
        digits_ = subtractMagnitudes(other.digits_, digits_);
        negative_ = other.negative_;
    }
    normalize();
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& other) { return *this += -other; }

BigInteger& BigInteger::operator*=(const BigInteger& other) {
    const bool resultNegative = negative_ != other.negative_;
    digits_ = multiplyMagnitudes(digits_, other.digits_);
    negative_ = resultNegative;
    normalize();
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
    if (other.isZero()) {
        throw std::domain_error("Деление на ноль");
    }
    const bool resultNegative = negative_ != other.negative_;
    digits_ = divideMagnitudes(digits_, other.digits_);
    negative_ = resultNegative;
    normalize();
    return *this;
}

BigInteger& BigInteger::operator++() { return *this += 1; }
BigInteger& BigInteger::operator--() { return *this -= 1; }

BigInteger BigInteger::operator++(int) {
    BigInteger previous(*this);
    ++*this;
    return previous;
}

BigInteger BigInteger::operator--(int) {
    BigInteger previous(*this);
    --*this;
    return previous;
}

void BigInteger::normalize() {
    trimLeadingZeros(digits_);
    if (digits_.empty()) {
        negative_ = false;
    }
}

BigInteger operator+(BigInteger lhs, const BigInteger& rhs) { return lhs += rhs; }
BigInteger operator-(BigInteger lhs, const BigInteger& rhs) { return lhs -= rhs; }
BigInteger operator*(BigInteger lhs, const BigInteger& rhs) { return lhs *= rhs; }
BigInteger operator/(BigInteger lhs, const BigInteger& rhs) { return lhs /= rhs; }

bool operator==(const BigInteger& lhs, const BigInteger& rhs) { return lhs.compareTo(rhs) == 0; }
bool operator!=(const BigInteger& lhs, const BigInteger& rhs) { return lhs.compareTo(rhs) != 0; }
bool operator<(const BigInteger& lhs, const BigInteger& rhs) { return lhs.compareTo(rhs) < 0; }
bool operator<=(const BigInteger& lhs, const BigInteger& rhs) { return lhs.compareTo(rhs) <= 0; }
bool operator>(const BigInteger& lhs, const BigInteger& rhs) { return lhs.compareTo(rhs) > 0; }
bool operator>=(const BigInteger& lhs, const BigInteger& rhs) { return lhs.compareTo(rhs) >= 0; }

std::ostream& operator<<(std::ostream& out, const BigInteger& value) {
    return out << value.toString();
}

std::istream& operator>>(std::istream& in, BigInteger& value) {
    std::string token;
    if (!(in >> token)) {
        return in;
    }
    try {
        value = BigInteger(token);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}

}  // namespace lab1
