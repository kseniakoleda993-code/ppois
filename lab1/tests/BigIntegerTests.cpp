/**
 * @file BigIntegerTests.cpp
 * @author Коледа Ксения
 * @brief Модульные тесты класса BigInteger.
 */
#include <gtest/gtest.h>

#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "BigInteger.h"

using lab1::BigInteger;

namespace {
constexpr long long kLongLongMax = std::numeric_limits<long long>::max();
constexpr long long kLongLongMin = std::numeric_limits<long long>::min();
const char* const kHugePositive = "123456789012345678901234567890";
const char* const kHugeSecond = "987654321098765432109876543210";

BigInteger factorial(int number) {
    BigInteger result(1);
    for (int factor = 2; factor <= number; ++factor) {
        result *= factor;
    }
    return result;
}
}  // namespace

TEST(BigIntegerConstruction, DefaultIsZero) {
    const BigInteger zero;
    EXPECT_EQ(zero.toString(), "0");
    EXPECT_TRUE(zero.isZero());
    EXPECT_FALSE(zero.isNegative());
}

TEST(BigIntegerConstruction, FromBuiltInInteger) {
    EXPECT_EQ(BigInteger(0).toString(), "0");
    EXPECT_EQ(BigInteger(42).toString(), "42");
    EXPECT_EQ(BigInteger(-42).toString(), "-42");
    EXPECT_EQ(BigInteger(1000000000LL).toString(), "1000000000");
}

TEST(BigIntegerConstruction, FromLimitsOfLongLong) {
    EXPECT_EQ(BigInteger(kLongLongMax).toString(), "9223372036854775807");
    EXPECT_EQ(BigInteger(kLongLongMin).toString(), "-9223372036854775808");
}

TEST(BigIntegerConstruction, FromStringWithSigns) {
    EXPECT_EQ(BigInteger(std::string(kHugePositive)).toString(), kHugePositive);
    EXPECT_EQ(BigInteger("-123").toString(), "-123");
    EXPECT_EQ(BigInteger("+5").toString(), "5");
}

TEST(BigIntegerConstruction, LeadingZerosAndNegativeZeroAreNormalized) {
    EXPECT_EQ(BigInteger("000123").toString(), "123");
    EXPECT_EQ(BigInteger("-0").toString(), "0");
    EXPECT_FALSE(BigInteger("-000").isNegative());
}

TEST(BigIntegerConstruction, InternalZerosAreKept) {
    EXPECT_EQ(BigInteger("1000000000").toString(), "1000000000");
    EXPECT_EQ(BigInteger("1000000001").toString(), "1000000001");
    EXPECT_EQ(BigInteger("1000000000000000000").toString(), "1000000000000000000");
}

TEST(BigIntegerConstruction, InvalidStringsThrow) {
    EXPECT_THROW(BigInteger(""), std::invalid_argument);
    EXPECT_THROW(BigInteger("-"), std::invalid_argument);
    EXPECT_THROW(BigInteger("12a"), std::invalid_argument);
    EXPECT_THROW(BigInteger(" 1"), std::invalid_argument);
    EXPECT_THROW(BigInteger("1.5"), std::invalid_argument);
}

TEST(BigIntegerConstruction, CopyAndAssignmentAreIndependent) {
    BigInteger original(kHugePositive);
    BigInteger copy(original);
    BigInteger assigned;
    assigned = original;
    original += 1;
    EXPECT_EQ(copy.toString(), kHugePositive);
    EXPECT_EQ(assigned.toString(), kHugePositive);
}

TEST(BigIntegerAddition, CarryPropagatesAcrossDigits) {
    EXPECT_EQ((BigInteger(999999999) + BigInteger(1)).toString(), "1000000000");
    EXPECT_EQ((BigInteger("999999999999999999") + 1).toString(), "1000000000000000000");
}

TEST(BigIntegerAddition, HugeNumbers) {
    const BigInteger sum = BigInteger(kHugePositive) + BigInteger(kHugeSecond);
    EXPECT_EQ(sum.toString(), "1111111110111111111011111111100");
}

TEST(BigIntegerAddition, MixedSigns) {
    EXPECT_EQ(BigInteger(5) + BigInteger(-7), BigInteger(-2));
    EXPECT_EQ(BigInteger(-5) + BigInteger(7), BigInteger(2));
    EXPECT_EQ(BigInteger(-5) + BigInteger(-7), BigInteger(-12));
}

TEST(BigIntegerAddition, OppositeNumbersGiveNonNegativeZero) {
    const BigInteger result = BigInteger(7) + BigInteger(-7);
    EXPECT_TRUE(result.isZero());
    EXPECT_FALSE(result.isNegative());
}

TEST(BigIntegerAddition, CompoundOperatorReturnsReferenceToSelf) {
    BigInteger value(1);
    BigInteger& result = (value += 2);
    EXPECT_TRUE(&result == &value);
    EXPECT_EQ(value, BigInteger(3));
}

TEST(BigIntegerAddition, WithBuiltInInteger) {
    const BigInteger value("1000000000000");
    EXPECT_EQ((value + 5).toString(), "1000000000005");
    EXPECT_EQ((5 + value).toString(), "1000000000005");
    BigInteger copy = value;
    copy += -1;
    EXPECT_EQ(copy.toString(), "999999999999");
}

TEST(BigIntegerSubtraction, BorrowPropagatesAcrossDigits) {
    EXPECT_EQ((BigInteger("1000000000000000000") - 1).toString(), "999999999999999999");
}

TEST(BigIntegerSubtraction, SignsAndZero) {
    EXPECT_EQ(BigInteger(5) - BigInteger(7), BigInteger(-2));
    EXPECT_EQ(BigInteger(-5) - BigInteger(-7), BigInteger(2));
    EXPECT_EQ(BigInteger(-5) - BigInteger(7), BigInteger(-12));
    EXPECT_FALSE((BigInteger(9) - BigInteger(9)).isNegative());
}

TEST(BigIntegerSubtraction, HugeNumbers) {
    const BigInteger difference = BigInteger(kHugeSecond) - BigInteger(kHugePositive);
    EXPECT_EQ(difference.toString(), "864197532086419753208641975320");
}

TEST(BigIntegerSubtraction, SelfSubtractionGivesZero) {
    BigInteger value(kHugePositive);
    value -= value;
    EXPECT_TRUE(value.isZero());
}

TEST(BigIntegerSubtraction, WithBuiltInInteger) {
    const BigInteger value(10);
    EXPECT_EQ(value - 25, BigInteger(-15));
    EXPECT_EQ(25 - value, BigInteger(15));
    BigInteger copy = value;
    copy -= 3;
    EXPECT_EQ(copy, BigInteger(7));
}

TEST(BigIntegerMultiplication, SmallAndBoundaryValues) {
    EXPECT_EQ((BigInteger(123456789) * BigInteger(987654321)).toString(), "121932631112635269");
    EXPECT_EQ((BigInteger("99999999999") * BigInteger("99999999999")).toString(), "9999999999800000000001");
}

TEST(BigIntegerMultiplication, HugeNumbers) {
    const BigInteger product = BigInteger(kHugePositive) * BigInteger(kHugeSecond);
    EXPECT_EQ(product.toString(), "121932631137021795226185032733622923332237463801111263526900");
}

TEST(BigIntegerMultiplication, SignRules) {
    EXPECT_EQ(BigInteger(-3) * BigInteger(4), BigInteger(-12));
    EXPECT_EQ(BigInteger(3) * BigInteger(-4), BigInteger(-12));
    EXPECT_EQ(BigInteger(-3) * BigInteger(-4), BigInteger(12));
}

TEST(BigIntegerMultiplication, ByZeroGivesNonNegativeZero) {
    const BigInteger result = BigInteger(-5) * 0;
    EXPECT_TRUE(result.isZero());
    EXPECT_FALSE(result.isNegative());
}

TEST(BigIntegerMultiplication, WithBuiltInInteger) {
    BigInteger value("1000000000000");
    EXPECT_EQ((value * 3).toString(), "3000000000000");
    EXPECT_EQ((-3 * value).toString(), "-3000000000000");
    value *= 10;
    EXPECT_EQ(value.toString(), "10000000000000");
}

TEST(BigIntegerMultiplication, FactorialsAndPowers) {
    EXPECT_EQ(factorial(25).toString(), "15511210043330985984000000");
    EXPECT_EQ(factorial(50).toString(), "30414093201713378043612608166064768844377641568960512000000000000");
    BigInteger power(1);
    for (int step = 0; step < 100; ++step) {
        power *= 2;
    }
    EXPECT_EQ(power.toString(), "1267650600228229401496703205376");
}

TEST(BigIntegerDivision, TruncatesTowardZero) {
    EXPECT_EQ(BigInteger(7) / BigInteger(2), BigInteger(3));
    EXPECT_EQ(BigInteger(-7) / BigInteger(2), BigInteger(-3));
    EXPECT_EQ(BigInteger(7) / BigInteger(-2), BigInteger(-3));
    EXPECT_EQ(BigInteger(-7) / BigInteger(-2), BigInteger(3));
}

TEST(BigIntegerDivision, SmallerDividendGivesNonNegativeZero) {
    const BigInteger result = BigInteger(-5) / BigInteger(7);
    EXPECT_TRUE(result.isZero());
    EXPECT_FALSE(result.isNegative());
    EXPECT_TRUE((BigInteger(0) / BigInteger(5)).isZero());
}

TEST(BigIntegerDivision, HugeNumbers) {
    EXPECT_EQ((BigInteger("1000000000000000000000000000007") / BigInteger("1000000000000000")).toString(),
              "1000000000000000");
    EXPECT_EQ((BigInteger(kHugeSecond) / BigInteger(kHugePositive)).toString(), "8");
    EXPECT_TRUE((BigInteger(kHugePositive) / BigInteger(kHugeSecond)).isZero());
}

TEST(BigIntegerDivision, FactorialQuotients) {
    EXPECT_EQ(factorial(30) / factorial(28), BigInteger(870));
    EXPECT_EQ((factorial(50) / factorial(25)).toString(), "1960781468160819415703172080467968000000");
}

TEST(BigIntegerDivision, ProductDividedByFactorGivesOtherFactor) {
    const BigInteger first(kHugePositive);
    const BigInteger second(kHugeSecond);
    EXPECT_EQ((first * second) / second, first);
    EXPECT_EQ((first * second) / first, second);
}

TEST(BigIntegerDivision, ByZeroThrows) {
    EXPECT_THROW(BigInteger(5) / BigInteger(0), std::domain_error);
    EXPECT_THROW(BigInteger(5) / 0, std::domain_error);
}

TEST(BigIntegerDivision, WithBuiltInInteger) {
    BigInteger value("1000000000000000000000");
    EXPECT_EQ((value / 1000).toString(), "1000000000000000000");
    value /= 10;
    EXPECT_EQ(value.toString(), "100000000000000000000");
}

TEST(BigIntegerIncrement, PrefixAndPostfixIncrement) {
    BigInteger value(5);
    EXPECT_EQ(++value, BigInteger(6));
    const BigInteger previous = value++;
    EXPECT_EQ(previous, BigInteger(6));
    EXPECT_EQ(value, BigInteger(7));
}

TEST(BigIntegerIncrement, PrefixAndPostfixDecrement) {
    BigInteger value(5);
    EXPECT_EQ(--value, BigInteger(4));
    const BigInteger previous = value--;
    EXPECT_EQ(previous, BigInteger(4));
    EXPECT_EQ(value, BigInteger(3));
}

TEST(BigIntegerIncrement, CrossesZeroAndDigitBoundary) {
    BigInteger value(0);
    --value;
    EXPECT_EQ(value, BigInteger(-1));
    ++value;
    EXPECT_TRUE(value.isZero());
    BigInteger boundary(999999999);
    ++boundary;
    EXPECT_EQ(boundary.toString(), "1000000000");
}

TEST(BigIntegerComparison, BetweenBigIntegers) {
    const BigInteger small(kHugePositive);
    const BigInteger large(kHugeSecond);
    EXPECT_TRUE(small < large);
    EXPECT_TRUE(small <= large);
    EXPECT_TRUE(large > small);
    EXPECT_TRUE(large >= small);
    EXPECT_TRUE(small != large);
    EXPECT_FALSE(small == large);
}

TEST(BigIntegerComparison, NegativeNumbersAreOrderedByValue) {
    EXPECT_TRUE(BigInteger(-10) < BigInteger(-2));
    EXPECT_TRUE(BigInteger(-2) < BigInteger(1));
    EXPECT_TRUE(BigInteger("-1000000000000") < BigInteger(-5));
    EXPECT_TRUE(BigInteger(-5) > BigInteger("-1000000000000"));
}

TEST(BigIntegerComparison, EqualNumbers) {
    EXPECT_TRUE(BigInteger("12345678901234567890") == BigInteger("12345678901234567890"));
    EXPECT_TRUE(BigInteger(3) <= BigInteger(3));
    EXPECT_TRUE(BigInteger(3) >= BigInteger(3));
    EXPECT_EQ(BigInteger(3).compareTo(BigInteger(3)), 0);
}

TEST(BigIntegerComparison, WithBuiltInInteger) {
    const BigInteger value(10);
    EXPECT_TRUE(value == 10);
    EXPECT_TRUE(value != 11);
    EXPECT_TRUE(value > 9);
    EXPECT_TRUE(value >= 10);
    EXPECT_TRUE(value < 11);
    EXPECT_TRUE(value <= 10);
    EXPECT_TRUE(9 < value);
}

TEST(BigIntegerConversion, ToLongLong) {
    EXPECT_EQ(static_cast<long long>(BigInteger(12345)), 12345LL);
    EXPECT_EQ(static_cast<long long>(BigInteger(-12345)), -12345LL);
    EXPECT_EQ(static_cast<long long>(BigInteger(0)), 0LL);
}

TEST(BigIntegerConversion, LimitsRoundTrip) {
    EXPECT_EQ(static_cast<long long>(BigInteger(kLongLongMax)), kLongLongMax);
    EXPECT_EQ(static_cast<long long>(BigInteger(kLongLongMin)), kLongLongMin);
}

TEST(BigIntegerConversion, OverflowThrows) {
    EXPECT_THROW(static_cast<long long>(BigInteger("9223372036854775808")), std::overflow_error);
    EXPECT_THROW(static_cast<long long>(BigInteger("-9223372036854775809")), std::overflow_error);
}

TEST(BigIntegerUnaryMinus, NegatesAndKeepsZeroNonNegative) {
    EXPECT_EQ(-BigInteger(5), BigInteger(-5));
    EXPECT_EQ(-BigInteger(-5), BigInteger(5));
    EXPECT_FALSE((-BigInteger(0)).isNegative());
}

TEST(BigIntegerStreams, OutputUsesDecimalNotation) {
    std::ostringstream out;
    out << BigInteger("-1000000000000000000000");
    EXPECT_EQ(out.str(), "-1000000000000000000000");
}

TEST(BigIntegerStreams, InputParsesWord) {
    std::istringstream in("  -123456789012345678901234567890  ");
    BigInteger value;
    in >> value;
    EXPECT_TRUE(static_cast<bool>(in));
    EXPECT_EQ(value.toString(), "-123456789012345678901234567890");
}

TEST(BigIntegerStreams, BadInputSetsFailbitAndKeepsValue) {
    std::istringstream in("abc");
    BigInteger value(77);
    in >> value;
    EXPECT_TRUE(in.fail());
    EXPECT_EQ(value, BigInteger(77));
}

TEST(BigIntegerStreams, EmptyInputFails) {
    std::istringstream in("");
    BigInteger value(1);
    in >> value;
    EXPECT_TRUE(in.fail());
    EXPECT_EQ(value, BigInteger(1));
}

TEST(BigIntegerRandomized, MatchesBuiltInArithmetic) {
    std::mt19937 generator(12345);
    std::uniform_int_distribution<long long> distribution(-1000000000LL, 1000000000LL);
    for (int attempt = 0; attempt < 500; ++attempt) {
        const long long left = distribution(generator);
        const long long right = distribution(generator);
        EXPECT_EQ(BigInteger(left) + BigInteger(right), BigInteger(left + right));
        EXPECT_EQ(BigInteger(left) - BigInteger(right), BigInteger(left - right));
        EXPECT_EQ(BigInteger(left) * BigInteger(right), BigInteger(left * right));
        EXPECT_EQ(BigInteger(left) < BigInteger(right), left < right);
        if (right != 0) {
            EXPECT_EQ(BigInteger(left) / BigInteger(right), BigInteger(left / right));
        }
    }
}
