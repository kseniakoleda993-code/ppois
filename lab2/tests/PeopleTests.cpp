#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(CertificateTest, ValidityAndDaysLeft) {
    const Certificate certificate("engine", Date(2025, 1, 1), Date(2027, 1, 1));
    EXPECT_TRUE(certificate.isValidOn(today()));
    EXPECT_FALSE(certificate.isValidOn(Date(2024, 1, 1)));
    EXPECT_FALSE(certificate.isValidOn(Date(2027, 1, 2)));
    EXPECT_EQ(certificate.daysLeft(Date(2026, 12, 22)), 10);
    EXPECT_EQ(certificate.daysLeft(Date(2028, 1, 1)), 0);
    EXPECT_EQ(certificate.getName(), std::string("engine"));
    EXPECT_TRUE(certificate.getIssuedOn() == Date(2025, 1, 1));
    EXPECT_TRUE(certificate.getValidUntil() == Date(2027, 1, 1));
}

TEST(PersonTest, AgeAndAdulthood) {
    Person person = makePerson("Иван", 2000);
    EXPECT_EQ(person.ageOn(Date(2026, 5, 20)), 26);
    EXPECT_EQ(person.ageOn(Date(2026, 5, 19)), 25);
    EXPECT_TRUE(person.isAdultOn(today()));
    EXPECT_FALSE(makePerson("Ребёнок", 2015).isAdultOn(today()));
    EXPECT_EQ(person.getName(), std::string("Иван"));
    EXPECT_TRUE(person.getBirthDate() == Date(2000, 5, 20));
}

TEST(PersonTest, MoveAndUpdateContact) {
    Person person = makePerson();
    person.moveTo(Address("ул. Новая 1", "Гомель", "246000"));
    person.updateContact(ContactInfo("+375 33 111-22-33", "new@mail.by"));
    EXPECT_TRUE(person.getAddress().isInCity("Гомель"));
    EXPECT_EQ(person.getContact().getEmail(), std::string("new@mail.by"));
}

TEST(ClientTest, ValidClient) {
    const Client client = makeClient(5);
    EXPECT_EQ(client.getId(), 5);
    EXPECT_TRUE(client.getRegisteredOn() == Date(2026, 1, 15));
    EXPECT_TRUE(client.getVehicleVins().empty());
    EXPECT_TRUE(client.getTotalSpent().isZero());
}

TEST(ClientTest, InvalidClientsAreRejected) {
    const Date date(2026, 1, 15);
    const Address address("a", "b", "220000");
    const ContactInfo goodContact("+375 29 123-45-67", "a@b.by");
    EXPECT_THROW(Client(1, Person("", Date(1990, 1, 1), goodContact, address), date), InvalidClientDataException);
    EXPECT_THROW(Client(1, Person("N", Date(1990, 1, 1), ContactInfo("1", "a@b.by"), address), date),
                 InvalidClientDataException);
    EXPECT_THROW(Client(1, Person("N", Date(1990, 1, 1), ContactInfo("+375 29 123-45-67", "bad"), address), date),
                 InvalidClientDataException);
    EXPECT_THROW(Client(1, Person("N", Date(2015, 1, 1), goodContact, address), date), InvalidClientDataException);
    EXPECT_NO_THROW(Client(1, Person("N", Date(1990, 1, 1), ContactInfo("+375 29 123-45-67", ""), address), date));
}

TEST(ClientTest, VehicleRegistration) {
    Client client = makeClient();
    Vehicle vehicle = makeVehicle();
    client.registerVehicle(vehicle);
    EXPECT_EQ(vehicle.getOwnerId(), 1);
    EXPECT_TRUE(client.ownsVehicle(vehicle));
    EXPECT_THROW(client.registerVehicle(vehicle), InvalidVehicleDataException);
    client.releaseVehicle(vehicle);
    EXPECT_FALSE(client.ownsVehicle(vehicle));
}

TEST(ClientTest, SpendingMakesRegularCustomer) {
    Client client = makeClient();
    EXPECT_FALSE(client.isRegularCustomer());
    client.recordSpending(Money::fromUnits(1000));
    EXPECT_TRUE(client.isRegularCustomer());
    EXPECT_EQ(client.getTotalSpent(), Money::fromUnits(1000));
}

TEST(EmployeeTest, ServiceAndSalary) {
    Employee employee = makeEmployee();
    EXPECT_EQ(employee.getEmployeeId(), 10);
    EXPECT_EQ(employee.getPosition(), std::string("Механик"));
    EXPECT_TRUE(employee.getHireDate() == Date(2020, 3, 1));
    EXPECT_EQ(employee.yearsOfServiceOn(today()), 6);
    EXPECT_TRUE(employee.isEligibleForBonusOn(today()));
    EXPECT_FALSE(employee.isEligibleForBonusOn(Date(2020, 6, 1)));
    employee.raiseSalary(10.0);
    EXPECT_EQ(employee.getMonthlySalary(), Money::fromUnits(2200));
    EXPECT_THROW(Employee(1, makePerson(), "x", today(), Money(-1)), InvalidMoneyOperationException);
}

TEST(MechanicTest, CertificatesAndSkills) {
    Mechanic mechanic = makeMechanic();
    EXPECT_EQ(mechanic.getSpecialization(), std::string("engine"));
    EXPECT_EQ(mechanic.getHourlyRate(), Money::fromUnits(30));
    EXPECT_TRUE(mechanic.canHandle(oilChange(), today()));
    EXPECT_FALSE(mechanic.canHandle(diagnostics(), today()));
    mechanic.addCertificate(Certificate("electrics", Date(2025, 1, 1), Date(2027, 1, 1)));
    EXPECT_TRUE(mechanic.hasValidCertificate("electrics", today()));
    EXPECT_TRUE(mechanic.canHandle(diagnostics(), today()));
    EXPECT_FALSE(mechanic.hasValidCertificate("electrics", Date(2028, 1, 1)));
}

TEST(MechanicTest, ReservationsAndLaborCost) {
    Mechanic mechanic = makeMechanic();
    const TimeSlot morning(today(), 9, 2);
    EXPECT_TRUE(mechanic.isAvailable(morning));
    mechanic.reserve(morning);
    EXPECT_FALSE(mechanic.isAvailable(TimeSlot(today(), 10, 2)));
    EXPECT_THROW(mechanic.reserve(TimeSlot(today(), 10, 1)), MechanicNotAvailableException);
    EXPECT_TRUE(mechanic.isAvailable(TimeSlot(today(), 12, 1)));
    EXPECT_EQ(mechanic.laborCost(2.5), Money::fromUnits(75));
}

TEST(ServiceAdvisorTest, CommissionAndRecommendations) {
    ServiceAdvisor advisor(makeEmployee(11, "Ольга Петрова"), 5.0);
    advisor.recordServedClient(makeClient());
    EXPECT_EQ(advisor.getCustomersServed(), 1);
    EXPECT_NEAR(advisor.getCommissionPercent(), 5.0, 1e-9);
    EXPECT_EQ(advisor.calculateCommission(Money::fromUnits(200)), Money::fromUnits(10));
    const Vehicle vehicle = makeVehicle();
    const std::vector<ServicePlan> plans = {ServicePlan("Масло", 10000, 100000), ServicePlan("Ремень", 60000, 100000)};
    const std::vector<std::string> due = advisor.recommendServices(plans, vehicle);
    EXPECT_EQ(due.size(), static_cast<std::size_t>(1));
    EXPECT_EQ(due.front(), std::string("Масло"));
}

TEST(ManagerTest, DiscountAndRefundLimits) {
    Manager manager(makeEmployee(12, "Сергей Орлов"), 15.0, Money::fromUnits(500));
    EXPECT_TRUE(manager.canApprove(makeDiscount(10.0)));
    EXPECT_FALSE(manager.canApprove(makeDiscount(20.0)));
    manager.approveDiscount(makeDiscount(10.0));
    EXPECT_EQ(manager.getApprovedCount(), 1);
    EXPECT_THROW(manager.approveDiscount(makeDiscount(20.0)), DiscountLimitExceededException);
    EXPECT_NO_THROW(manager.authorizeRefund(Money::fromUnits(500)));
    EXPECT_THROW(manager.authorizeRefund(Money::fromUnits(501)), InsufficientFundsException);
    manager.raiseLimit(100.0);
    EXPECT_NEAR(manager.getDiscountLimitPercent(), 50.0, 1e-9);
    EXPECT_EQ(manager.getRefundLimit(), Money::fromUnits(500));
}

TEST(CashierTest, CashOperations) {
    Cashier cashier(makeEmployee(13, "Мария Ким"), Money::fromUnits(100));
    cashier.acceptCash(Money::fromUnits(50));
    EXPECT_EQ(cashier.getCashDrawer(), Money::fromUnits(150));
    EXPECT_EQ(cashier.giveChange(Money::fromUnits(30), Money::fromUnits(50)), Money::fromUnits(20));
    EXPECT_EQ(cashier.getCashDrawer(), Money::fromUnits(180));
    EXPECT_THROW(cashier.giveChange(Money::fromUnits(30), Money::fromUnits(10)), InsufficientFundsException);
    cashier.withdraw(Money::fromUnits(80));
    EXPECT_THROW(cashier.withdraw(Money::fromUnits(1000)), InsufficientFundsException);
    cashier.registerReceipt();
    EXPECT_EQ(cashier.getReceiptsIssued(), 1);
}

TEST(WarehouseKeeperTest, DeliveryAndIssue) {
    WarehouseKeeper keeper(makeEmployee(14, "Антон Белов"), "A");
    PartsInventory inventory("Склад");
    inventory.registerPart(oilFilter(), 1, 2);
    PurchaseOrder order(1, makeSupplier(), today());
    order.addLine(OrderLine(oilFilter(), 10));
    EXPECT_THROW(keeper.receiveDelivery(inventory, order), InvalidOrderStateException);
    order.send();
    keeper.receiveDelivery(inventory, order);
    EXPECT_EQ(inventory.quantityOf("F-001"), 11);
    keeper.issueParts(inventory, OrderLine(oilFilter(), 9));
    EXPECT_EQ(keeper.getOperationsCount(), 2);
    EXPECT_EQ(keeper.buildReorderList(inventory).size(), static_cast<std::size_t>(1));
    EXPECT_EQ(keeper.getZone(), std::string("A"));
}
