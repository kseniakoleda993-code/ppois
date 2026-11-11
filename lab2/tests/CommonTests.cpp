#include <gtest/gtest.h>

#include "autoshop/Common.h"

using namespace autoshop;

TEST(MoneyTest, FromUnitsAndCents) {
    const Money money = Money::fromUnits(12);
    EXPECT_EQ(money.getCents(), 1200LL);
    EXPECT_EQ(money.getCurrency(), std::string("BYN"));
}

TEST(MoneyTest, PlusAndMinus) {
    EXPECT_EQ(Money(150).plus(Money(250)), Money(400));
    EXPECT_EQ(Money(150).minus(Money(250)), Money(-100));
}

TEST(MoneyTest, DifferentCurrenciesAreRejected) {
    EXPECT_THROW(Money(1, "BYN").plus(Money(1, "USD")), InvalidMoneyOperationException);
    EXPECT_THROW(Money(1, "BYN").minus(Money(1, "USD")), InvalidMoneyOperationException);
    EXPECT_THROW(Money(1, "BYN").isGreaterOrEqual(Money(1, "USD")), InvalidMoneyOperationException);
}

TEST(MoneyTest, MultiplyAndPercent) {
    EXPECT_EQ(Money(1000).multiply(2.5), Money(2500));
    EXPECT_EQ(Money(1000).applyPercent(15.0), Money(150));
}

TEST(MoneyTest, SignAndComparison) {
    EXPECT_TRUE(Money(-1).isNegative());
    EXPECT_FALSE(Money(1).isNegative());
    EXPECT_TRUE(Money(0).isZero());
    EXPECT_TRUE(Money(5).isGreaterOrEqual(Money(5)));
    EXPECT_FALSE(Money(4).isGreaterOrEqual(Money(5)));
}

TEST(MoneyTest, ToStringHandlesSignAndCents) {
    EXPECT_EQ(Money(1234).toString(), std::string("12.34 BYN"));
    EXPECT_EQ(Money(-5).toString(), std::string("-0.05 BYN"));
}

TEST(DateTest, DefaultAndAccessors) {
    const Date date;
    EXPECT_EQ(date.getYear(), 2000);
    const Date other(2026, 11, 10);
    EXPECT_EQ(other.getMonth(), 11);
    EXPECT_EQ(other.getDay(), 10);
}

TEST(DateTest, InvalidDatesAreRejected) {
    EXPECT_THROW(Date(2026, 13, 1), InvalidDateException);
    EXPECT_THROW(Date(2026, 0, 1), InvalidDateException);
    EXPECT_THROW(Date(2026, 4, 31), InvalidDateException);
    EXPECT_THROW(Date(1800, 1, 1), InvalidDateException);
    EXPECT_THROW(Date(2023, 2, 29), InvalidDateException);
}

TEST(DateTest, LeapYears) {
    EXPECT_TRUE(Date(2024, 2, 29).isLeapYear());
    EXPECT_FALSE(Date(2100, 1, 1).isLeapYear());
    EXPECT_TRUE(Date(2000, 1, 1).isLeapYear());
    EXPECT_EQ(Date(2024, 2, 1).daysInMonth(), 29);
    EXPECT_EQ(Date(2023, 2, 1).daysInMonth(), 28);
}

TEST(DateTest, PlusDaysCrossesMonthsAndYears) {
    EXPECT_EQ(Date(2026, 1, 30).plusDays(3), Date(2026, 2, 2));
    EXPECT_EQ(Date(2026, 12, 31).plusDays(1), Date(2027, 1, 1));
    EXPECT_EQ(Date(2024, 2, 28).plusDays(1), Date(2024, 2, 29));
    EXPECT_THROW(Date(2026, 1, 1).plusDays(-1), InvalidDateException);
}

TEST(DateTest, DaysUntilWorksBothWays) {
    EXPECT_EQ(Date(2026, 1, 1).daysUntil(Date(2026, 1, 11)), 10);
    EXPECT_EQ(Date(2026, 1, 11).daysUntil(Date(2026, 1, 1)), -10);
    EXPECT_EQ(Date(2026, 1, 1).daysUntil(Date(2026, 1, 1)), 0);
}

TEST(DateTest, OrderingAndFormatting) {
    EXPECT_TRUE(Date(2026, 1, 1).isBefore(Date(2026, 1, 2)));
    EXPECT_FALSE(Date(2026, 1, 2).isBefore(Date(2026, 1, 2)));
    EXPECT_EQ(Date(2026, 3, 5).toString(), std::string("2026-03-05"));
}

TEST(TimeSlotTest, ValidSlot) {
    const TimeSlot slot(Date(2026, 11, 10), 9, 2);
    EXPECT_EQ(slot.endHour(), 11);
    EXPECT_EQ(slot.getStartHour(), 9);
    EXPECT_EQ(slot.getDurationHours(), 2);
    EXPECT_TRUE(slot.isOn(Date(2026, 11, 10)));
    EXPECT_TRUE(slot.getDate() == Date(2026, 11, 10));
}

TEST(TimeSlotTest, InvalidSlotsAreRejected) {
    EXPECT_THROW(TimeSlot(Date(), 7, 1), InvalidDateException);
    EXPECT_THROW(TimeSlot(Date(), 19, 2), InvalidDateException);
    EXPECT_THROW(TimeSlot(Date(), 10, 0), InvalidDateException);
}

TEST(TimeSlotTest, ContainsAndOverlaps) {
    const TimeSlot slot(Date(2026, 11, 10), 9, 2);
    EXPECT_TRUE(slot.contains(9));
    EXPECT_FALSE(slot.contains(11));
    EXPECT_TRUE(slot.overlaps(TimeSlot(Date(2026, 11, 10), 10, 2)));
    EXPECT_FALSE(slot.overlaps(TimeSlot(Date(2026, 11, 10), 11, 1)));
    EXPECT_FALSE(slot.overlaps(TimeSlot(Date(2026, 11, 11), 9, 2)));
}

TEST(AddressTest, FullAddressAndCity) {
    const Address address("ул. Мира 5", "Минск", "220001");
    EXPECT_EQ(address.fullAddress(), std::string("220001, Минск, ул. Мира 5"));
    EXPECT_TRUE(address.isInCity("Минск"));
    EXPECT_FALSE(address.isInCity("Брест"));
    EXPECT_EQ(address.getStreet(), std::string("ул. Мира 5"));
    EXPECT_EQ(address.getCity(), std::string("Минск"));
}

TEST(AddressTest, PostalCodeValidation) {
    EXPECT_TRUE(Address("a", "b", "220001").hasValidPostalCode());
    EXPECT_FALSE(Address("a", "b", "2200").hasValidPostalCode());
    EXPECT_FALSE(Address("a", "b", "22000x").hasValidPostalCode());
    EXPECT_EQ(Address("a", "b", "220001").getPostalCode(), std::string("220001"));
}

TEST(ContactInfoTest, PhoneDigitsAndValidation) {
    const ContactInfo contact("+375 (29) 123-45-67", "a@b.by");
    EXPECT_EQ(contact.phoneDigits(), std::string("375291234567"));
    EXPECT_TRUE(contact.hasValidPhone());
    EXPECT_FALSE(ContactInfo("12345", "").hasValidPhone());
    EXPECT_EQ(contact.getPhone(), std::string("+375 (29) 123-45-67"));
}

TEST(ContactInfoTest, EmailValidation) {
    EXPECT_TRUE(ContactInfo("", "user@mail.com").hasValidEmail());
    EXPECT_FALSE(ContactInfo("", "usermail.com").hasValidEmail());
    EXPECT_FALSE(ContactInfo("", "@mail.com").hasValidEmail());
    EXPECT_FALSE(ContactInfo("", "user@mailcom").hasValidEmail());
    EXPECT_FALSE(ContactInfo("", "user@.com").hasValidEmail());
    EXPECT_FALSE(ContactInfo("", "user@mail.").hasValidEmail());
    EXPECT_EQ(ContactInfo("", "user@mail.com").getEmail(), std::string("user@mail.com"));
}

TEST(ContactInfoTest, MaskedPhone) {
    EXPECT_EQ(ContactInfo("+375291234567", "").maskedPhone(), std::string("********4567"));
    EXPECT_EQ(ContactInfo("123", "").maskedPhone(), std::string("123"));
}
