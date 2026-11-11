#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
struct Fixture {
    Fixture() : shop(makeShop()) {
        clientId = shop.registerClient(makePerson(), today());
        shop.registerVehicle(clientId, makeVehicle());
        mechanicId = shop.hireMechanic(makePerson("Пётр Сидоров", 1985), "engine", Money::fromUnits(30), Date(2020, 1, 1));
    }
    int completeOrder(const std::string& serviceCode, double hours) {
        const int orderId = shop.openWorkOrder(clientId, kVin, today());
        shop.addServiceToOrder(orderId, serviceCode);
        shop.startWorkOrder(orderId, mechanicId);
        shop.completeWorkOrder(orderId, hours, today());
        return orderId;
    }
    AutoShop shop;
    int clientId = 0;
    int mechanicId = 0;
};
}  // namespace

TEST(AutoShopTest, ConstructionAndCatalog) {
    const AutoShop shop = makeShop();
    EXPECT_EQ(shop.getName(), std::string("Автомастерская «Гараж»"));
    EXPECT_TRUE(shop.getAddress().isInCity("Минск"));
    EXPECT_EQ(shop.getHourlyRate(), Money::fromUnits(30));
    EXPECT_EQ(shop.getBays().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(shop.getCatalog().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(shop.getInventory().quantityOf("F-001"), 10);
    EXPECT_EQ(shop.getAppointments().getCapacity(), 2);
    EXPECT_EQ(shop.findService("OIL").getName(), std::string("Замена масла"));
    EXPECT_THROW(shop.findService("NOPE"), NotFoundException);
}

TEST(AutoShopTest, RegistrationOfPeopleAndVehicles) {
    Fixture f;
    EXPECT_EQ(f.shop.getClients().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.shop.getVehicles().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.shop.getMechanics().size(), static_cast<std::size_t>(1));
    EXPECT_TRUE(f.shop.hasVehicle(kVin));
    EXPECT_FALSE(f.shop.hasVehicle(kSecondVin));
    EXPECT_EQ(f.shop.findVehicle(kVin).getOwnerId(), f.clientId);
    EXPECT_EQ(f.shop.findClient(f.clientId).getVehicleVins().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.shop.findMechanic(f.mechanicId).getSpecialization(), std::string("engine"));
    EXPECT_THROW(f.shop.registerVehicle(f.clientId, makeVehicle()), InvalidVehicleDataException);
    EXPECT_THROW(f.shop.registerVehicle(999, makeVehicle(kSecondVin)), NotFoundException);
    EXPECT_THROW(f.shop.findClient(999), NotFoundException);
    EXPECT_THROW(f.shop.findVehicle("X"), NotFoundException);
    EXPECT_THROW(f.shop.findMechanic(999), NotFoundException);
}

TEST(AutoShopTest, FullRepairAndPaymentFlow) {
    Fixture f;
    const int orderId = f.shop.openWorkOrder(f.clientId, kVin, today());
    f.shop.addServiceToOrder(orderId, "OIL");
    f.shop.addPartToOrder(orderId, "F-001", 1);
    f.shop.startWorkOrder(orderId, f.mechanicId);
    EXPECT_FALSE(f.shop.getBays().front().isFree());
    f.shop.completeWorkOrder(orderId, 1.0, today());
    EXPECT_TRUE(f.shop.getBays().front().isFree());
    EXPECT_EQ(f.shop.findOrder(orderId).getStatus(), WorkOrderStatus::Completed);
    EXPECT_EQ(f.shop.getInventory().quantityOf("F-001"), 9);
    const int invoiceId = f.shop.issueInvoice(orderId, today());
    EXPECT_EQ(f.shop.findInvoice(invoiceId).getAmount(), Money::fromUnits(65));
    f.shop.payInvoice(invoiceId, Money::fromUnits(65), "card", today());
    EXPECT_TRUE(f.shop.findInvoice(invoiceId).isPaid());
    EXPECT_EQ(f.shop.findClient(f.clientId).getTotalSpent(), Money::fromUnits(65));
    EXPECT_EQ(f.shop.getInvoices().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.shop.getOrders().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.shop.getNotifications().countFor(f.shop.findClient(f.clientId)), 2);
}

TEST(AutoShopTest, ReportAndExpenses) {
    Fixture f;
    const int orderId = f.completeOrder("OIL", 1.0);
    const int invoiceId = f.shop.issueInvoice(orderId, today());
    f.shop.payInvoice(invoiceId, Money::fromUnits(50), "cash", today());
    f.shop.recordExpense(ExpenseRecord("Аренда", Money::fromUnits(20), today(), ""));
    const FinancialReport report = f.shop.buildReport(Date(2026, 11, 1), Date(2026, 11, 30));
    EXPECT_EQ(report.getRevenue(), Money::fromUnits(50));
    EXPECT_EQ(report.profit(), Money::fromUnits(30));
}

TEST(AutoShopTest, LoyaltyDiscountAppliesAfterBigPurchase) {
    Fixture f;
    const int firstOrder = f.completeOrder("ENGINE", 10.0);
    const int firstInvoice = f.shop.issueInvoice(firstOrder, today());
    EXPECT_EQ(f.shop.findInvoice(firstInvoice).getAmount(), Money::fromUnits(1500));
    f.shop.payInvoice(firstInvoice, Money::fromUnits(1500), "transfer", today());
    const int secondOrder = f.completeOrder("OIL", 1.0);
    const int secondInvoice = f.shop.issueInvoice(secondOrder, today());
    EXPECT_EQ(f.shop.findInvoice(secondInvoice).getAmount(), Money(4850));
}

TEST(AutoShopTest, AppointmentsRespectBayCapacity) {
    Fixture f;
    const TimeSlot slot(today(), 9, 1);
    f.shop.bookAppointment(f.clientId, kVin, slot, "OIL");
    f.shop.bookAppointment(f.clientId, kVin, slot, "OIL");
    EXPECT_THROW(f.shop.bookAppointment(f.clientId, kVin, slot, "OIL"), AppointmentConflictException);
    EXPECT_EQ(f.shop.getAppointments().getAppointments().size(), static_cast<std::size_t>(2));
    EXPECT_THROW(f.shop.bookAppointment(f.clientId, kVin, slot, "NOPE"), NotFoundException);
}

TEST(AutoShopTest, ErrorsInOrderProcessing) {
    Fixture f;
    const int orderId = f.shop.openWorkOrder(f.clientId, kVin, today());
    EXPECT_THROW(f.shop.addPartToOrder(orderId, "B-010", 2), OutOfStockException);
    EXPECT_THROW(f.shop.addPartToOrder(orderId, "NOPE", 1), NotFoundException);
    EXPECT_THROW(f.shop.addServiceToOrder(999, "OIL"), NotFoundException);
    EXPECT_THROW(f.shop.startWorkOrder(orderId, 999), NotFoundException);
    EXPECT_THROW(f.shop.issueInvoice(orderId, today()), InvalidOrderStateException);
    EXPECT_THROW(f.shop.payInvoice(999, Money(1), "cash", today()), NotFoundException);
    EXPECT_THROW(f.shop.openWorkOrder(f.clientId, kSecondVin, today()), NotFoundException);
}

TEST(AutoShopTest, NoFreeBayStopsNewWork) {
    Fixture f;
    for (int count = 0; count < 2; ++count) {
        const int id = f.shop.openWorkOrder(f.clientId, kVin, today());
        f.shop.startWorkOrder(id, f.mechanicId);
    }
    const int third = f.shop.openWorkOrder(f.clientId, kVin, today());
    EXPECT_THROW(f.shop.startWorkOrder(third, f.mechanicId), BayNotAvailableException);
}
