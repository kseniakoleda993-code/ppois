#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(PartTest, StoresDataAndMarkup) {
    const Part part = oilFilter();
    EXPECT_EQ(part.getSku(), std::string("F-001"));
    EXPECT_EQ(part.getName(), std::string("Масляный фильтр"));
    EXPECT_EQ(part.getCategory(), std::string("filters"));
    EXPECT_EQ(part.getPrice(), Money::fromUnits(15));
    EXPECT_EQ(part.getCompatibleMakes().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(part.priceWithMarkup(20.0), Money::fromUnits(18));
}

TEST(PartTest, Compatibility) {
    const Vehicle golf = makeVehicle();
    EXPECT_TRUE(oilFilter().isCompatibleWith(golf));
    EXPECT_TRUE(brakePad().isCompatibleWith(golf));
    const Part toyotaOnly("T-1", "Фильтр", "filters", Money(100), {"Toyota"});
    EXPECT_FALSE(toyotaOnly.isCompatibleWith(golf));
}

TEST(PartTest, InvalidDataIsRejected) {
    EXPECT_THROW(Part("", "n", "c", Money(1)), InvalidPartDataException);
    EXPECT_THROW(Part("s", "", "c", Money(1)), InvalidPartDataException);
    EXPECT_THROW(Part("s", "n", "c", Money(-1)), InvalidPartDataException);
}

TEST(SupplierTest, DeliveryAndRating) {
    Supplier supplier = makeSupplier();
    EXPECT_TRUE(supplier.estimateDelivery(Date(2026, 11, 10)) == Date(2026, 11, 13));
    EXPECT_TRUE(supplier.isReliable());
    supplier.rate(3.0);
    EXPECT_FALSE(supplier.isReliable());
    EXPECT_THROW(supplier.rate(6.0), InvalidPartDataException);
    EXPECT_THROW(supplier.rate(-1.0), InvalidPartDataException);
    EXPECT_EQ(supplier.getName(), std::string("АвтоЗапчасть"));
    EXPECT_EQ(supplier.getLeadTimeDays(), 3);
    EXPECT_NEAR(supplier.getRating(), 3.0, 1e-9);
    EXPECT_TRUE(supplier.getContact().hasValidEmail());
}

TEST(OrderLineTest, TotalAndIncrease) {
    OrderLine line(oilFilter(), 3);
    EXPECT_EQ(line.lineTotal(), Money::fromUnits(45));
    line.increase(2);
    EXPECT_EQ(line.getQuantity(), 5);
    EXPECT_EQ(line.getPart().getSku(), std::string("F-001"));
    EXPECT_THROW(line.increase(0), InvalidPartDataException);
    EXPECT_THROW(OrderLine(oilFilter(), 0), InvalidPartDataException);
}

TEST(StockRecordTest, LowLevelAndRemoval) {
    StockRecord record(oilFilter(), 3, 2, "A1");
    EXPECT_FALSE(record.isLow());
    EXPECT_EQ(record.restockAmount(), 0);
    record.remove(1);
    EXPECT_TRUE(record.isLow());
    EXPECT_EQ(record.restockAmount(), 2);
    record.add(5);
    EXPECT_EQ(record.getQuantity(), 7);
    EXPECT_THROW(record.remove(100), OutOfStockException);
    EXPECT_EQ(record.getShelf(), std::string("A1"));
    EXPECT_EQ(record.getMinLevel(), 2);
    EXPECT_EQ(record.getPart().getSku(), std::string("F-001"));
}

TEST(PartsInventoryTest, RegisterReceiveIssue) {
    PartsInventory inventory("Склад");
    inventory.registerPart(oilFilter(), 5, 2);
    inventory.registerPart(oilFilter(), 3, 2);
    EXPECT_EQ(inventory.quantityOf("F-001"), 8);
    inventory.receive("F-001", 2);
    inventory.issue(OrderLine(oilFilter(), 4));
    EXPECT_EQ(inventory.quantityOf("F-001"), 6);
    EXPECT_EQ(inventory.getLocation(), std::string("Склад"));
}

TEST(PartsInventoryTest, AvailabilityAndErrors) {
    PartsInventory inventory("Склад");
    inventory.registerPart(oilFilter(), 2, 1);
    EXPECT_TRUE(inventory.isAvailable(OrderLine(oilFilter(), 2)));
    EXPECT_FALSE(inventory.isAvailable(OrderLine(oilFilter(), 3)));
    EXPECT_FALSE(inventory.isAvailable(OrderLine(brakePad(), 1)));
    EXPECT_THROW(inventory.receive("NOPE", 1), NotFoundException);
    EXPECT_THROW(inventory.issue(OrderLine(brakePad(), 1)), NotFoundException);
    EXPECT_THROW(inventory.find("NOPE"), NotFoundException);
    EXPECT_THROW(inventory.issue(OrderLine(oilFilter(), 5)), OutOfStockException);
}

TEST(PartsInventoryTest, LowStockAndValue) {
    PartsInventory inventory("Склад");
    inventory.registerPart(oilFilter(), 10, 2);
    inventory.registerPart(brakePad(), 1, 2);
    EXPECT_EQ(inventory.lowStockSkus().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(inventory.lowStockSkus().front(), std::string("B-010"));
    EXPECT_EQ(inventory.totalValue(), Money::fromUnits(210));
}

TEST(PurchaseOrderTest, LifecycleAndTotals) {
    PurchaseOrder order(1, makeSupplier(), Date(2026, 11, 10));
    order.addLine(OrderLine(oilFilter(), 10));
    EXPECT_EQ(order.total(), Money::fromUnits(150));
    EXPECT_TRUE(order.expectedDelivery() == Date(2026, 11, 13));
    order.send();
    EXPECT_EQ(order.getStatus(), PurchaseStatus::Sent);
    order.markReceived();
    EXPECT_EQ(order.getStatus(), PurchaseStatus::Received);
    EXPECT_EQ(order.getId(), 1);
    EXPECT_EQ(order.getLines().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(order.getSupplier().getName(), std::string("АвтоЗапчасть"));
    EXPECT_TRUE(order.getOrderedOn() == Date(2026, 11, 10));
}

TEST(PurchaseOrderTest, InvalidTransitions) {
    PurchaseOrder order(1, makeSupplier(), Date(2026, 11, 10));
    EXPECT_THROW(order.markReceived(), InvalidOrderStateException);
    order.send();
    EXPECT_THROW(order.send(), InvalidOrderStateException);
    EXPECT_THROW(order.addLine(OrderLine(oilFilter(), 1)), InvalidOrderStateException);
    order.markReceived();
    EXPECT_THROW(order.cancel(), InvalidOrderStateException);
}

TEST(PurchaseOrderTest, CancelDraft) {
    PurchaseOrder order(2, makeSupplier(), Date(2026, 11, 10));
    order.cancel();
    EXPECT_EQ(order.getStatus(), PurchaseStatus::Cancelled);
}

TEST(ToolTest, Calibration) {
    Tool tool("Динамометрический ключ", "SN-1", Date(2026, 1, 1), 180);
    EXPECT_TRUE(tool.nextCalibrationDate() == Date(2026, 6, 30));
    EXPECT_TRUE(tool.needsCalibrationOn(Date(2026, 7, 1)));
    EXPECT_FALSE(tool.needsCalibrationOn(Date(2026, 3, 1)));
    tool.calibrate(Date(2026, 7, 1));
    EXPECT_FALSE(tool.needsCalibrationOn(Date(2026, 8, 1)));
    EXPECT_EQ(tool.getName(), std::string("Динамометрический ключ"));
    EXPECT_EQ(tool.getSerialNumber(), std::string("SN-1"));
    EXPECT_EQ(tool.getIntervalDays(), 180);
    EXPECT_TRUE(tool.getLastCalibration() == Date(2026, 7, 1));
}
