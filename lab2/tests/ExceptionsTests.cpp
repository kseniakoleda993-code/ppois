#include <gtest/gtest.h>

#include <string>

#include "autoshop/Exceptions.h"

using namespace autoshop;

namespace {
template <typename Exception>
bool isCaughtAsBaseWithDetails(const std::string& details) {
    try {
        throw Exception(details);
    } catch (const AutoShopException& error) {
        return std::string(error.what()).find(details) != std::string::npos;
    }
    return false;
}
}  // namespace

TEST(ExceptionsTest, BaseExceptionKeepsMessage) {
    const AutoShopException error("сообщение");
    EXPECT_EQ(std::string(error.what()), "сообщение");
}

TEST(ExceptionsTest, EveryExceptionDerivesFromBaseAndKeepsDetails) {
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidClientDataException>("a"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidVehicleDataException>("b"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidMoneyOperationException>("c"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InsufficientFundsException>("d"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<OutOfStockException>("e"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidPartDataException>("f"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidServiceDataException>("g"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<MechanicNotAvailableException>("h"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<BayNotAvailableException>("i"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<AppointmentConflictException>("j"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidOrderStateException>("k"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidDateException>("l"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<PromoCodeException>("m"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<WarrantyExpiredException>("n"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<DiscountLimitExceededException>("o"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<NotFoundException>("p"));
}
