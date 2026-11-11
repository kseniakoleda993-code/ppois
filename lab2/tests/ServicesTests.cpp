#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(ServiceTypeTest, StoresDataAndComputesPrice) {
    const ServiceType service = oilChange();
    EXPECT_EQ(service.getCode(), std::string("OIL"));
    EXPECT_EQ(service.getName(), std::string("Замена масла"));
    EXPECT_EQ(service.getRequiredSkill(), std::string("engine"));
    EXPECT_EQ(service.getBasePrice(), Money::fromUnits(20));
    EXPECT_NEAR(service.getStandardHours(), 1.0, 1e-9);
    EXPECT_EQ(service.totalPrice(Money::fromUnits(30)), Money::fromUnits(50));
}

TEST(ServiceTypeTest, LongRunningAndSkill) {
    EXPECT_TRUE(engineOverhaul().isLongRunning());
    EXPECT_FALSE(oilChange().isLongRunning());
    EXPECT_TRUE(oilChange().requiresSkill("engine"));
    EXPECT_FALSE(oilChange().requiresSkill("electrics"));
}

TEST(ServiceTypeTest, InvalidDataIsRejected) {
    EXPECT_THROW(ServiceType("", "n", Money(1), 1.0, "s"), InvalidServiceDataException);
    EXPECT_THROW(ServiceType("c", "", Money(1), 1.0, "s"), InvalidServiceDataException);
    EXPECT_THROW(ServiceType("c", "n", Money(1), 0.0, "s"), InvalidServiceDataException);
    EXPECT_THROW(ServiceType("c", "n", Money(-1), 1.0, "s"), InvalidServiceDataException);
}

TEST(DiscountTest, ValidityAndAmounts) {
    const Discount discount = makeDiscount(10.0);
    EXPECT_TRUE(discount.isValidOn(Date(2026, 11, 10)));
    EXPECT_FALSE(discount.isValidOn(Date(2026, 12, 1)));
    EXPECT_FALSE(discount.isValidOn(Date(2026, 10, 31)));
    EXPECT_EQ(discount.savedAmount(Money::fromUnits(100)), Money::fromUnits(10));
    EXPECT_EQ(discount.applyTo(Money::fromUnits(100)), Money::fromUnits(90));
    EXPECT_NEAR(discount.getPercent(), 10.0, 1e-9);
    EXPECT_EQ(discount.getReason(), std::string("Акция"));
    EXPECT_TRUE(discount.getValidFrom() == Date(2026, 11, 1));
    EXPECT_TRUE(discount.getValidTo() == Date(2026, 11, 30));
}

TEST(DiscountTest, InvalidParametersAreRejected) {
    EXPECT_THROW(makeDiscount(0.0), DiscountLimitExceededException);
    EXPECT_THROW(makeDiscount(75.0), DiscountLimitExceededException);
    EXPECT_THROW(Discount(5.0, "x", Date(2026, 11, 2), Date(2026, 11, 1)), InvalidDateException);
}

TEST(DiscountTest, ExtendOnlyForward) {
    Discount discount = makeDiscount();
    discount.extendTo(Date(2026, 12, 31));
    EXPECT_TRUE(discount.isValidOn(Date(2026, 12, 15)));
    EXPECT_THROW(discount.extendTo(Date(2026, 11, 1)), InvalidDateException);
}

TEST(PromoCodeTest, RedeemUntilLimit) {
    PromoCode promo("WINTER", makeDiscount(), 2);
    EXPECT_EQ(promo.getCode(), std::string("WINTER"));
    EXPECT_TRUE(promo.canBeUsedOn(today()));
    EXPECT_NEAR(promo.redeem(today()).getPercent(), 10.0, 1e-9);
    promo.redeem(today());
    EXPECT_EQ(promo.remainingUses(), 0);
    EXPECT_EQ(promo.getTimesUsed(), 2);
    EXPECT_THROW(promo.redeem(today()), PromoCodeException);
}

TEST(PromoCodeTest, ExpiredCodeIsRejected) {
    PromoCode promo("OLD", makeDiscount(), 5);
    EXPECT_FALSE(promo.canBeUsedOn(Date(2027, 1, 1)));
    EXPECT_THROW(promo.redeem(Date(2027, 1, 1)), PromoCodeException);
    EXPECT_NEAR(promo.getDiscount().getPercent(), 10.0, 1e-9);
}
