#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
Invoice makeInvoice(const Garage& garage, int id = 1, long long units = 100) {
    return Invoice(id, makeCompletedOrder(garage), Money::fromUnits(units), today());
}
}  // namespace

TEST(InvoiceTest, CreationRules) {
    Garage garage;
    const Invoice invoice = makeInvoice(garage);
    EXPECT_EQ(invoice.getId(), 1);
    EXPECT_EQ(invoice.getWorkOrderId(), 100);
    EXPECT_EQ(invoice.getClientId(), 1);
    EXPECT_TRUE(invoice.getIssuedOn() == today());
    EXPECT_TRUE(invoice.getDueOn() == Date(2026, 11, 20));
    EXPECT_EQ(invoice.getAmount(), Money::fromUnits(100));
    EXPECT_TRUE(invoice.getPaid().isZero());
    EXPECT_THROW(Invoice(2, makeCompletedOrder(garage), Money(-1), today()), InvalidMoneyOperationException);
    const WorkOrder open(3, garage.client, garage.vehicle, today());
    EXPECT_THROW(Invoice(3, open, Money(1), today()), InvalidOrderStateException);
}

TEST(InvoiceTest, PaymentsAndOverdue) {
    Garage garage;
    Invoice invoice = makeInvoice(garage);
    invoice.registerPayment(Money::fromUnits(40));
    EXPECT_EQ(invoice.outstanding(), Money::fromUnits(60));
    EXPECT_FALSE(invoice.isPaid());
    EXPECT_THROW(invoice.registerPayment(Money::fromUnits(61)), InvalidMoneyOperationException);
    EXPECT_FALSE(invoice.isOverdueOn(Date(2026, 11, 20)));
    EXPECT_TRUE(invoice.isOverdueOn(Date(2026, 11, 25)));
    EXPECT_EQ(invoice.daysOverdue(Date(2026, 11, 25)), 5);
    EXPECT_EQ(invoice.daysOverdue(today()), 0);
    invoice.registerPayment(Money::fromUnits(60));
    EXPECT_TRUE(invoice.isPaid());
    EXPECT_FALSE(invoice.isOverdueOn(Date(2026, 12, 31)));
}

TEST(InvoiceTest, DiscountRules) {
    Garage garage;
    Invoice invoice = makeInvoice(garage);
    invoice.applyDiscount(makeDiscount(10.0), today());
    EXPECT_EQ(invoice.getAmount(), Money::fromUnits(90));
    EXPECT_THROW(invoice.applyDiscount(makeDiscount(), Date(2027, 1, 1)), InvalidDateException);
    invoice.registerPayment(Money::fromUnits(10));
    EXPECT_THROW(invoice.applyDiscount(makeDiscount(), today()), InvalidOrderStateException);
}

TEST(PaymentTest, ValidPayments) {
    Garage garage;
    const Invoice invoice = makeInvoice(garage);
    const Payment card(1, invoice, Money::fromUnits(100), "card", today());
    EXPECT_FALSE(card.isCash());
    EXPECT_EQ(card.processingFee(), Money(150));
    EXPECT_EQ(card.netAmount(), Money(9850));
    const Payment cash(2, invoice, Money::fromUnits(50), "cash", today());
    EXPECT_TRUE(cash.isCash());
    EXPECT_TRUE(cash.processingFee().isZero());
    EXPECT_EQ(cash.getId(), 2);
    EXPECT_EQ(cash.getInvoiceId(), 1);
    EXPECT_EQ(cash.getAmount(), Money::fromUnits(50));
    EXPECT_EQ(cash.getMethod(), std::string("cash"));
    EXPECT_TRUE(cash.getPaidOn() == today());
}

TEST(PaymentTest, InvalidPaymentsAreRejected) {
    Garage garage;
    const Invoice invoice = makeInvoice(garage);
    EXPECT_THROW(Payment(1, invoice, Money(0), "cash", today()), InvalidMoneyOperationException);
    EXPECT_THROW(Payment(1, invoice, Money(-5), "cash", today()), InvalidMoneyOperationException);
    EXPECT_THROW(Payment(1, invoice, Money::fromUnits(101), "cash", today()), InvalidMoneyOperationException);
    EXPECT_THROW(Payment(1, invoice, Money::fromUnits(10), "bitcoin", today()), InvalidMoneyOperationException);
    EXPECT_NO_THROW(Payment(1, invoice, Money::fromUnits(10), "transfer", today()));
}

TEST(ReceiptTest, RenderContainsKeyData) {
    Garage garage;
    const Invoice invoice = makeInvoice(garage);
    const Payment payment(7, invoice, Money::fromUnits(25), "cash", today());
    const Cashier cashier(makeEmployee(13, "Мария Ким"), Money(0));
    const Receipt receipt(3, payment, cashier);
    EXPECT_EQ(receipt.getNumber(), 3);
    EXPECT_EQ(receipt.getPaymentId(), 7);
    EXPECT_EQ(receipt.getTotal(), Money::fromUnits(25));
    EXPECT_EQ(receipt.getCashierName(), std::string("Мария Ким"));
    EXPECT_TRUE(receipt.getIssuedOn() == today());
    EXPECT_NE(receipt.render().find("25.00 BYN"), std::string::npos);
    EXPECT_NE(receipt.render().find("Мария Ким"), std::string::npos);
}

TEST(RefundTest, AmountLimitsAndShare) {
    Garage garage;
    const Invoice invoice = makeInvoice(garage);
    const Payment payment(1, invoice, Money::fromUnits(100), "cash", today());
    const Refund partial(1, payment, Money::fromUnits(25), "Брак");
    EXPECT_FALSE(partial.isFullRefundOf(payment));
    EXPECT_NEAR(partial.refundedSharePercent(payment), 25.0, 1e-9);
    EXPECT_TRUE(Refund(2, payment, Money::fromUnits(100), "Отмена").isFullRefundOf(payment));
    EXPECT_THROW(Refund(3, payment, Money::fromUnits(101), "x"), InsufficientFundsException);
    EXPECT_EQ(partial.getId(), 1);
    EXPECT_EQ(partial.getPaymentId(), 1);
    EXPECT_EQ(partial.getAmount(), Money::fromUnits(25));
    EXPECT_EQ(partial.getReason(), std::string("Брак"));
}

TEST(PayrollRecordTest, NetAndTaxRate) {
    const PayrollRecord record(makeEmployee(), "2026-11", Money::fromUnits(500), 13.0);
    EXPECT_EQ(record.getGross(), Money::fromUnits(2500));
    EXPECT_EQ(record.getTax(), Money::fromUnits(325));
    EXPECT_EQ(record.net(), Money::fromUnits(2175));
    EXPECT_NEAR(record.effectiveTaxPercent(), 13.0, 1e-9);
    EXPECT_EQ(record.getEmployeeId(), 10);
    EXPECT_EQ(record.getPeriod(), std::string("2026-11"));
    EXPECT_EQ(record.getBonus(), Money::fromUnits(500));
    const Employee unpaid(1, makePerson(), "x", today(), Money(0));
    EXPECT_NEAR(PayrollRecord(unpaid, "p", Money(0), 13.0).effectiveTaxPercent(), 0.0, 1e-9);
}

TEST(ExpenseRecordTest, MonthAndSize) {
    const ExpenseRecord rent("Аренда", Money::fromUnits(800), today(), "Ноябрь");
    EXPECT_TRUE(rent.isInMonth(2026, 11));
    EXPECT_FALSE(rent.isInMonth(2026, 10));
    EXPECT_TRUE(rent.isLarge());
    EXPECT_FALSE(ExpenseRecord("Вода", Money::fromUnits(20), today(), "").isLarge());
    EXPECT_EQ(rent.getCategory(), std::string("Аренда"));
    EXPECT_EQ(rent.getAmount(), Money::fromUnits(800));
    EXPECT_TRUE(rent.getDate() == today());
    EXPECT_EQ(rent.getDescription(), std::string("Ноябрь"));
}

TEST(FinancialReportTest, RevenueExpensesAndProfit) {
    Garage garage;
    Invoice invoice = makeInvoice(garage);
    invoice.registerPayment(Money::fromUnits(100));
    FinancialReport report(Date(2026, 11, 1), Date(2026, 11, 30));
    EXPECT_NEAR(report.profitMarginPercent(), 0.0, 1e-9);
    EXPECT_FALSE(report.isProfitable());
    report.addInvoice(invoice);
    report.addExpense(ExpenseRecord("Аренда", Money::fromUnits(40), today(), ""));
    report.addExpense(ExpenseRecord("Старый", Money::fromUnits(999), Date(2026, 1, 1), ""));
    EXPECT_EQ(report.getRevenue(), Money::fromUnits(100));
    EXPECT_EQ(report.getExpenses(), Money::fromUnits(40));
    EXPECT_EQ(report.profit(), Money::fromUnits(60));
    EXPECT_TRUE(report.isProfitable());
    EXPECT_NEAR(report.profitMarginPercent(), 60.0, 1e-9);
    EXPECT_TRUE(report.includes(today()));
    EXPECT_TRUE(report.getPeriodStart() == Date(2026, 11, 1));
    EXPECT_TRUE(report.getPeriodEnd() == Date(2026, 11, 30));
    FinancialReport other(Date(2026, 1, 1), Date(2026, 1, 31));
    other.addInvoice(invoice);
    EXPECT_TRUE(other.getRevenue().isZero());
    EXPECT_THROW(FinancialReport(Date(2026, 2, 1), Date(2026, 1, 1)), InvalidDateException);
}

TEST(AccountantTest, PayrollAndReports) {
    Garage garage;
    Invoice invoice = makeInvoice(garage);
    invoice.registerPayment(Money::fromUnits(100));
    Accountant accountant(makeEmployee(15, "Елена Лис"), 13.0);
    const PayrollRecord record = accountant.calculatePayroll(makeEmployee(), "2026-11", Money(0));
    EXPECT_EQ(record.getTax(), Money::fromUnits(260));
    const FinancialReport report = accountant.compileReport({invoice}, {ExpenseRecord("A", Money::fromUnits(10), today(), "")},
                                                            Date(2026, 11, 1), Date(2026, 11, 30));
    EXPECT_EQ(report.profit(), Money::fromUnits(90));
    EXPECT_EQ(accountant.getReportsPrepared(), 1);
    EXPECT_NEAR(accountant.getTaxPercent(), 13.0, 1e-9);
}

TEST(ShiftTest, ValidationAndProperties) {
    const Employee employee = makeEmployee();
    const Shift day(employee, today(), 8, 8);
    EXPECT_EQ(day.endHour(), 16);
    EXPECT_FALSE(day.isNightShift());
    EXPECT_TRUE(Shift(employee, today(), 22, 8).isNightShift());
    EXPECT_TRUE(Shift(employee, today(), 3, 4).isNightShift());
    EXPECT_TRUE(day.overlaps(Shift(employee, today(), 15, 4)));
    EXPECT_FALSE(day.overlaps(Shift(employee, today(), 16, 4)));
    EXPECT_FALSE(day.overlaps(Shift(makeEmployee(11), today(), 8, 8)));
    EXPECT_THROW(Shift(employee, today(), 24, 1), InvalidDateException);
    EXPECT_THROW(Shift(employee, today(), 8, 0), InvalidDateException);
    EXPECT_THROW(Shift(employee, today(), 8, 13), InvalidDateException);
    EXPECT_EQ(day.getEmployeeId(), 10);
    EXPECT_TRUE(day.getDate() == today());
    EXPECT_EQ(day.getStartHour(), 8);
    EXPECT_EQ(day.getHours(), 8);
}

TEST(WorkScheduleTest, ShiftsAndHours) {
    const Employee employee = makeEmployee();
    WorkSchedule schedule(40);
    schedule.addShift(Shift(employee, today(), 8, 8));
    schedule.addShift(Shift(employee, Date(2026, 11, 11), 8, 8));
    EXPECT_THROW(schedule.addShift(Shift(employee, today(), 10, 2)), AppointmentConflictException);
    EXPECT_EQ(schedule.hoursFor(employee), 16);
    EXPECT_EQ(schedule.hoursFor(makeEmployee(11)), 0);
    EXPECT_EQ(schedule.shiftsOn(today()).size(), static_cast<std::size_t>(1));
    EXPECT_TRUE(schedule.isOnDuty(employee, today(), 9));
    EXPECT_FALSE(schedule.isOnDuty(employee, today(), 18));
    EXPECT_TRUE(schedule.canAddHours(employee, 24));
    EXPECT_FALSE(schedule.canAddHours(employee, 25));
    EXPECT_EQ(schedule.getMaxTotalHours(), 40);
    EXPECT_EQ(schedule.getShifts().size(), static_cast<std::size_t>(2));
}

TEST(LoyaltyCardTest, PointsTiersAndDiscounts) {
    LoyaltyCard card(makeClient(), today());
    EXPECT_EQ(card.tier(), std::string("Bronze"));
    EXPECT_NEAR(card.discountPercent(), 0.0, 1e-9);
    card.earn(Money::fromUnits(1000));
    EXPECT_EQ(card.getPoints(), 100);
    EXPECT_EQ(card.tier(), std::string("Silver"));
    EXPECT_NEAR(card.discountPercent(), 3.0, 1e-9);
    card.earn(Money::fromUnits(4000));
    EXPECT_EQ(card.tier(), std::string("Gold"));
    EXPECT_NEAR(card.discountPercent(), 5.0, 1e-9);
    card.redeem(100);
    EXPECT_EQ(card.getPoints(), 400);
    EXPECT_THROW(card.redeem(1000), InsufficientFundsException);
    EXPECT_EQ(card.getClientId(), 1);
    EXPECT_TRUE(card.getIssuedOn() == today());
}

TEST(NotificationTest, ChannelAndData) {
    const Notification note(1, "Привет", "email", today());
    EXPECT_TRUE(note.isEmail());
    EXPECT_FALSE(Notification(1, "x", "sms", today()).isEmail());
    EXPECT_EQ(note.getRecipientId(), 1);
    EXPECT_EQ(note.getMessage(), std::string("Привет"));
    EXPECT_EQ(note.getChannel(), std::string("email"));
    EXPECT_TRUE(note.getCreatedOn() == today());
}

TEST(NotificationServiceTest, SendsMessagesThroughRightChannel) {
    Garage garage;
    NotificationService service;
    service.notifyAppointment(garage.client, Appointment(1, garage.client, garage.vehicle, TimeSlot(today(), 9, 1), oilChange()));
    service.notifyInvoice(garage.client, makeInvoice(garage));
    service.notifyWorkReady(garage.client, makeCompletedOrder(garage), today());
    EXPECT_EQ(service.countFor(garage.client), 3);
    EXPECT_EQ(service.countFor(makeClient(2)), 0);
    EXPECT_TRUE(service.getSent().front().isEmail());
    const Client smsClient(3, Person("Без почты", Date(1990, 1, 1), ContactInfo("+375 29 123-45-67", ""),
                                     Address("a", "b", "220000")), today());
    service.notifyWorkReady(smsClient, makeCompletedOrder(garage), today());
    EXPECT_FALSE(service.getSent().back().isEmail());
}
