#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
Appointment makeAppointment(const Garage& garage, int id, int startHour, int hours = 1) {
    return Appointment(id, garage.client, garage.vehicle, TimeSlot(today(), startHour, hours), oilChange());
}
}  // namespace

TEST(AppointmentTest, ValidAndNotOwnedVehicle) {
    Garage garage;
    const Appointment appointment = makeAppointment(garage, 1, 9);
    EXPECT_EQ(appointment.getId(), 1);
    EXPECT_EQ(appointment.getClientId(), 1);
    EXPECT_EQ(appointment.getVin(), std::string(kVin));
    EXPECT_EQ(appointment.getService().getCode(), std::string("OIL"));
    EXPECT_EQ(appointment.getStatus(), AppointmentStatus::Scheduled);
    EXPECT_EQ(appointment.getSlot().getStartHour(), 9);
    const Vehicle stranger = makeVehicle(kSecondVin);
    EXPECT_THROW(Appointment(2, garage.client, stranger, TimeSlot(today(), 9, 1), oilChange()),
                 InvalidVehicleDataException);
}

TEST(AppointmentTest, StatusTransitions) {
    Garage garage;
    Appointment appointment = makeAppointment(garage, 1, 9);
    EXPECT_THROW(appointment.complete(), InvalidOrderStateException);
    appointment.confirm();
    EXPECT_THROW(appointment.confirm(), InvalidOrderStateException);
    appointment.complete();
    EXPECT_EQ(appointment.getStatus(), AppointmentStatus::Completed);
    EXPECT_THROW(appointment.cancel(), InvalidOrderStateException);
    EXPECT_FALSE(appointment.isActive());
}

TEST(AppointmentTest, RescheduleAndConflicts) {
    Garage garage;
    Appointment first = makeAppointment(garage, 1, 9, 2);
    Appointment second = makeAppointment(garage, 2, 10, 1);
    EXPECT_TRUE(first.conflictsWith(second));
    first.reschedule(TimeSlot(today(), 14, 1));
    EXPECT_FALSE(first.conflictsWith(second));
    second.cancel();
    EXPECT_EQ(second.getStatus(), AppointmentStatus::Cancelled);
    EXPECT_THROW(second.reschedule(TimeSlot(today(), 9, 1)), InvalidOrderStateException);
}

TEST(AppointmentBookTest, BookingRespectsCapacity) {
    Garage garage;
    AppointmentBook book(1);
    book.book(makeAppointment(garage, 1, 9, 2));
    EXPECT_EQ(book.countActiveAt(TimeSlot(today(), 10, 1)), 1);
    EXPECT_FALSE(book.isSlotFree(TimeSlot(today(), 10, 1)));
    EXPECT_THROW(book.book(makeAppointment(garage, 2, 10)), AppointmentConflictException);
    book.book(makeAppointment(garage, 3, 12));
    EXPECT_EQ(book.getAppointments().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(book.getCapacity(), 1);
}

TEST(AppointmentBookTest, FindCancelConfirm) {
    Garage garage;
    AppointmentBook book(2);
    book.book(makeAppointment(garage, 1, 9));
    book.confirm(1);
    EXPECT_EQ(book.findById(1).getStatus(), AppointmentStatus::Confirmed);
    book.cancel(1);
    EXPECT_EQ(book.findById(1).getStatus(), AppointmentStatus::Cancelled);
    EXPECT_THROW(book.findById(99), NotFoundException);
    EXPECT_THROW(book.cancel(99), NotFoundException);
    EXPECT_EQ(book.appointmentsOn(today()).size(), static_cast<std::size_t>(1));
    EXPECT_TRUE(book.appointmentsOn(Date(2026, 11, 11)).empty());
}

TEST(AppointmentBookTest, FindFreeSlot) {
    Garage garage;
    AppointmentBook book(1);
    book.book(makeAppointment(garage, 1, 8, 2));
    const auto slot = book.findFreeSlot(today(), 2);
    EXPECT_TRUE(slot.has_value());
    EXPECT_EQ(slot->getStartHour(), 10);
    AppointmentBook full(1);
    full.book(makeAppointment(garage, 2, 8, 12));
    EXPECT_FALSE(full.findFreeSlot(today(), 1).has_value());
}

TEST(EstimateTest, TotalsAndApproval) {
    Garage garage;
    Estimate estimate(1, garage.client, Date(2026, 11, 20), 10.0);
    estimate.addService(oilChange());
    estimate.addPart(OrderLine(oilFilter(), 1));
    EXPECT_EQ(estimate.subtotal(Money::fromUnits(30)), Money::fromUnits(65));
    EXPECT_EQ(estimate.total(Money::fromUnits(30)), Money(7150));
    EXPECT_FALSE(estimate.isApproved());
    estimate.approve(today());
    EXPECT_TRUE(estimate.isApproved());
    EXPECT_FALSE(estimate.isExpiredOn(today()));
    EXPECT_EQ(estimate.getId(), 1);
    EXPECT_EQ(estimate.getClientId(), 1);
    EXPECT_EQ(estimate.getServices().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(estimate.getParts().size(), static_cast<std::size_t>(1));
    EXPECT_NEAR(estimate.getMarkupPercent(), 10.0, 1e-9);
    EXPECT_TRUE(estimate.getValidUntil() == Date(2026, 11, 20));
}

TEST(EstimateTest, ExpiredEstimateCannotBeApproved) {
    Garage garage;
    Estimate estimate(2, garage.client, Date(2026, 11, 1), 0.0);
    EXPECT_TRUE(estimate.isExpiredOn(today()));
    EXPECT_THROW(estimate.approve(today()), InvalidOrderStateException);
}

TEST(WorkOrderTest, CreationRequiresOwnership) {
    Garage garage;
    const WorkOrder order(1, garage.client, garage.vehicle, today());
    EXPECT_EQ(order.getStatus(), WorkOrderStatus::Opened);
    EXPECT_EQ(order.getClientId(), 1);
    EXPECT_EQ(order.getVin(), std::string(kVin));
    EXPECT_TRUE(order.getOpenedOn() == today());
    EXPECT_THROW(WorkOrder(2, garage.client, makeVehicle(kSecondVin), today()), InvalidOrderStateException);
}

TEST(WorkOrderTest, FullLifecycleAndTotals) {
    Garage garage;
    WorkOrder order(1, garage.client, garage.vehicle, today());
    order.addService(oilChange());
    order.addPart(OrderLine(oilFilter(), 2));
    EXPECT_NEAR(order.plannedHours(), 1.0, 1e-9);
    EXPECT_EQ(order.servicesTotal(), Money::fromUnits(20));
    EXPECT_EQ(order.partsTotal(), Money::fromUnits(30));
    EXPECT_EQ(order.laborTotal(Money::fromUnits(30)), Money::fromUnits(30));
    EXPECT_EQ(order.total(Money::fromUnits(30)), Money::fromUnits(80));
    order.assignMechanic(makeMechanic());
    order.start();
    order.complete(2.0);
    EXPECT_EQ(order.getStatus(), WorkOrderStatus::Completed);
    EXPECT_NEAR(order.getLaborHours(), 2.0, 1e-9);
    EXPECT_EQ(order.getMechanicId(), 10);
    EXPECT_EQ(order.total(Money::fromUnits(30)), Money::fromUnits(110));
    EXPECT_EQ(order.getServices().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(order.getParts().size(), static_cast<std::size_t>(1));
}

TEST(WorkOrderTest, InvalidTransitions) {
    Garage garage;
    WorkOrder order(1, garage.client, garage.vehicle, today());
    EXPECT_THROW(order.start(), InvalidOrderStateException);
    EXPECT_THROW(order.complete(1.0), InvalidOrderStateException);
    order.assignMechanic(makeMechanic());
    order.start();
    EXPECT_THROW(order.assignMechanic(makeMechanic()), InvalidOrderStateException);
    EXPECT_THROW(order.complete(0.0), InvalidOrderStateException);
    order.complete(1.0);
    EXPECT_THROW(order.addService(oilChange()), InvalidOrderStateException);
    EXPECT_THROW(order.addPart(OrderLine(oilFilter(), 1)), InvalidOrderStateException);
    EXPECT_THROW(order.cancel(), InvalidOrderStateException);
}

TEST(WorkOrderTest, CancelAndDiscount) {
    Garage garage;
    WorkOrder order(1, garage.client, garage.vehicle, today());
    order.addService(oilChange());
    order.applyDiscount(makeDiscount(10.0), today());
    EXPECT_NEAR(order.getDiscountPercent(), 10.0, 1e-9);
    EXPECT_EQ(order.total(Money::fromUnits(30)), Money::fromUnits(45));
    EXPECT_THROW(order.applyDiscount(makeDiscount(), Date(2027, 1, 1)), InvalidDateException);
    order.cancel();
    EXPECT_EQ(order.getStatus(), WorkOrderStatus::Cancelled);
}

TEST(WorkshopBayTest, OccupyReleaseAndTools) {
    Garage garage;
    const WorkOrder order = makeCompletedOrder(garage, 5);
    WorkshopBay bay(1, "lift");
    EXPECT_TRUE(bay.isFree());
    bay.occupy(order);
    EXPECT_EQ(bay.getOccupiedByOrderId(), 5);
    EXPECT_THROW(bay.occupy(order), BayNotAvailableException);
    bay.release();
    EXPECT_TRUE(bay.isFree());
    bay.addTool(Tool("Ключ", "SN", Date(2026, 1, 1), 100));
    EXPECT_TRUE(bay.hasTool("Ключ"));
    EXPECT_FALSE(bay.hasTool("Домкрат"));
    EXPECT_EQ(bay.toolsNeedingCalibration(today()).size(), static_cast<std::size_t>(1));
    EXPECT_TRUE(bay.toolsNeedingCalibration(Date(2026, 2, 1)).empty());
    EXPECT_EQ(bay.getNumber(), 1);
    EXPECT_EQ(bay.getType(), std::string("lift"));
    EXPECT_EQ(bay.getTools().size(), static_cast<std::size_t>(1));
}

TEST(QualityCheckTest, ChecklistResults) {
    Garage garage;
    const WorkOrder order = makeCompletedOrder(garage);
    QualityCheck check(order, makeMechanic(11), today());
    EXPECT_FALSE(check.hasPassed());
    EXPECT_NEAR(check.passRatePercent(), 0.0, 1e-9);
    check.recordItem("Уровень масла", true);
    EXPECT_TRUE(check.hasPassed());
    check.recordItem("Течи", false);
    EXPECT_FALSE(check.hasPassed());
    EXPECT_EQ(check.failedItems().size(), static_cast<std::size_t>(1));
    EXPECT_NEAR(check.passRatePercent(), 50.0, 1e-9);
    EXPECT_EQ(check.getWorkOrderId(), 100);
    EXPECT_EQ(check.getInspectorId(), 11);
    EXPECT_TRUE(check.getCheckedOn() == today());
    EXPECT_EQ(check.getItems().size(), static_cast<std::size_t>(2));
}

TEST(InspectionReportTest, AggregatesFindings) {
    InspectionReport report(makeVehicle(), makeMechanic(), today());
    EXPECT_EQ(report.worstSeverity(), 0);
    EXPECT_FALSE(report.hasCriticalIssues());
    report.addResult(DiagnosticResult("A", "a", 2, Money::fromUnits(100)));
    report.addResult(DiagnosticResult("B", "b", 5, Money::fromUnits(250)));
    EXPECT_TRUE(report.hasCriticalIssues());
    EXPECT_EQ(report.worstSeverity(), 5);
    EXPECT_EQ(report.totalEstimatedCost(), Money::fromUnits(350));
    EXPECT_EQ(report.getVin(), std::string(kVin));
    EXPECT_EQ(report.getMechanicId(), 10);
    EXPECT_TRUE(report.getDate() == today());
    EXPECT_EQ(report.getResults().size(), static_cast<std::size_t>(2));
}

TEST(WarrantyClaimTest, ApproveAndReject) {
    Garage garage;
    const WorkOrder order = makeCompletedOrder(garage);
    WarrantyClaim claim(1, order, Warranty("W", Date(2027, 1, 1), 200000), "Течь масла");
    EXPECT_FALSE(claim.isResolved());
    claim.approve(garage.vehicle, today());
    EXPECT_EQ(claim.getStatus(), ClaimStatus::Approved);
    WarrantyClaim late(2, order, Warranty("W", Date(2026, 1, 1), 200000), "Поздно");
    EXPECT_THROW(late.approve(garage.vehicle, today()), WarrantyExpiredException);
    EXPECT_EQ(late.getStatus(), ClaimStatus::Rejected);
    WarrantyClaim manual(3, order, Warranty("W", Date(2027, 1, 1), 1), "x");
    manual.reject();
    EXPECT_TRUE(manual.isResolved());
    EXPECT_EQ(claim.getId(), 1);
    EXPECT_EQ(claim.getWorkOrderId(), 100);
    EXPECT_EQ(claim.getReason(), std::string("Течь масла"));
    EXPECT_EQ(claim.getWarranty().getMileageLimitKm(), 200000);
}

TEST(ComplaintTest, LifecycleAndOverdue) {
    Garage garage;
    const WorkOrder order = makeCompletedOrder(garage);
    Complaint complaint(1, garage.client, order, "Долго ждал", Date(2026, 10, 1));
    EXPECT_EQ(complaint.getStatus(), ComplaintStatus::Open);
    EXPECT_EQ(complaint.ageInDaysOn(today()), 40);
    EXPECT_TRUE(complaint.isOverdueOn(today()));
    complaint.escalate();
    EXPECT_EQ(complaint.getStatus(), ComplaintStatus::Escalated);
    complaint.resolve(Manager(makeEmployee(12), 10.0, Money(1)), "Компенсация");
    EXPECT_EQ(complaint.getStatus(), ComplaintStatus::Resolved);
    EXPECT_EQ(complaint.getResolvedById(), 12);
    EXPECT_FALSE(complaint.isOverdueOn(today()));
    EXPECT_THROW(complaint.escalate(), InvalidOrderStateException);
    EXPECT_EQ(complaint.getId(), 1);
    EXPECT_EQ(complaint.getClientId(), 1);
    EXPECT_EQ(complaint.getWorkOrderId(), 100);
    EXPECT_EQ(complaint.getText(), std::string("Долго ждал"));
    EXPECT_TRUE(complaint.getOpenedOn() == Date(2026, 10, 1));
}

TEST(CustomerReviewTest, RatingValidationAndSummary) {
    Garage garage;
    const WorkOrder order = makeCompletedOrder(garage);
    const CustomerReview good(garage.client, order, 5, "Отлично");
    EXPECT_TRUE(good.isPositive());
    EXPECT_EQ(good.summary(), std::string("5/5: Отлично"));
    EXPECT_FALSE(CustomerReview(garage.client, order, 2, "Плохо").isPositive());
    EXPECT_THROW(CustomerReview(garage.client, order, 0, "x"), InvalidClientDataException);
    EXPECT_THROW(CustomerReview(garage.client, order, 6, "x"), InvalidClientDataException);
    EXPECT_EQ(good.getClientId(), 1);
    EXPECT_EQ(good.getWorkOrderId(), 100);
    EXPECT_EQ(good.getRating(), 5);
    EXPECT_EQ(good.getComment(), std::string("Отлично"));
}
