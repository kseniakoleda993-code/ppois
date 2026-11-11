#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(EngineTest, TypeAndPerformance) {
    const Engine diesel("diesel", 2.0, 300);
    EXPECT_TRUE(diesel.isDiesel());
    EXPECT_TRUE(diesel.isHighPerformance());
    EXPECT_EQ(diesel.oilChangeIntervalKm(), 10000);
    const Engine petrol("petrol", 1.6, 105);
    EXPECT_FALSE(petrol.isDiesel());
    EXPECT_FALSE(petrol.isHighPerformance());
    EXPECT_EQ(petrol.oilChangeIntervalKm(), 15000);
    EXPECT_EQ(petrol.getType(), std::string("petrol"));
    EXPECT_NEAR(petrol.getVolumeLiters(), 1.6, 1e-9);
    EXPECT_EQ(petrol.getPowerHp(), 105);
}

TEST(TireSetTest, WearAndAge) {
    TireSet tires("205/55R16", "winter", 70.0, 2022);
    EXPECT_FALSE(tires.needsReplacement());
    tires.addWear(15.0);
    EXPECT_TRUE(tires.needsReplacement());
    tires.addWear(50.0);
    EXPECT_NEAR(tires.getWearPercent(), 100.0, 1e-9);
    EXPECT_EQ(tires.ageInYears(2026), 4);
    EXPECT_EQ(tires.getSize(), std::string("205/55R16"));
    EXPECT_EQ(tires.getSeason(), std::string("winter"));
    EXPECT_EQ(tires.getProductionYear(), 2022);
}

TEST(InsuranceTest, ValidityCoverageAndExtension) {
    Insurance policy("P-1", "Белгосстрах", Date(2026, 12, 31), Money::fromUnits(5000));
    EXPECT_TRUE(policy.isValidOn(today()));
    EXPECT_FALSE(policy.isValidOn(Date(2027, 1, 1)));
    EXPECT_TRUE(policy.covers(Money::fromUnits(4000)));
    EXPECT_FALSE(policy.covers(Money::fromUnits(6000)));
    policy.extend(10);
    EXPECT_TRUE(policy.isValidOn(Date(2027, 1, 5)));
    EXPECT_EQ(policy.getPolicyNumber(), std::string("P-1"));
    EXPECT_EQ(policy.getProvider(), std::string("Белгосстрах"));
    EXPECT_TRUE(policy.getValidUntil() == Date(2027, 1, 10));
    EXPECT_EQ(policy.getCoverageLimit(), Money::fromUnits(5000));
}

TEST(WarrantyTest, ActiveByDateAndMileage) {
    Warranty warranty("Гарантия на двигатель", Date(2027, 1, 1), 150000);
    EXPECT_TRUE(warranty.isActive(100000, today()));
    EXPECT_FALSE(warranty.isActive(160000, today()));
    EXPECT_FALSE(warranty.isActive(100000, Date(2027, 2, 1)));
    EXPECT_EQ(warranty.remainingKm(100000), 50000);
    EXPECT_EQ(warranty.remainingKm(200000), 0);
    warranty.prolong(31);
    EXPECT_TRUE(warranty.isActive(100000, Date(2027, 2, 1)));
    EXPECT_EQ(warranty.getDescription(), std::string("Гарантия на двигатель"));
    EXPECT_EQ(warranty.getMileageLimitKm(), 150000);
    EXPECT_TRUE(warranty.getCoveredUntil() == Date(2027, 2, 1));
}

TEST(VehicleTest, StoresDataAndDefaults) {
    const Vehicle vehicle = makeVehicle();
    EXPECT_EQ(vehicle.getVin(), std::string(kVin));
    EXPECT_EQ(vehicle.getPlate(), std::string("1234 AB-7"));
    EXPECT_EQ(vehicle.getMake(), std::string("Volkswagen"));
    EXPECT_EQ(vehicle.getModel(), std::string("Golf"));
    EXPECT_EQ(vehicle.getYear(), 2015);
    EXPECT_EQ(vehicle.getMileage(), 120000);
    EXPECT_EQ(vehicle.getOwnerId(), 0);
    EXPECT_EQ(vehicle.getEngine().getType(), std::string("petrol"));
    EXPECT_EQ(vehicle.getTires().getSize(), std::string("205/55R16"));
}

TEST(VehicleTest, InvalidDataIsRejected) {
    const Engine engine("petrol", 1.6, 100);
    const TireSet tires("a", "b", 0.0, 2020);
    EXPECT_THROW(Vehicle("short", "1", "m", "m", 2015, 0, engine, tires), InvalidVehicleDataException);
    EXPECT_THROW(Vehicle(kVin, "", "m", "m", 2015, 0, engine, tires), InvalidVehicleDataException);
    EXPECT_THROW(Vehicle(kVin, "1", "m", "m", 1900, 0, engine, tires), InvalidVehicleDataException);
    EXPECT_THROW(Vehicle(kVin, "1", "m", "m", 2015, -1, engine, tires), InvalidVehicleDataException);
}

TEST(VehicleTest, MileageChanges) {
    Vehicle vehicle = makeVehicle();
    vehicle.driveKm(500);
    EXPECT_EQ(vehicle.getMileage(), 120500);
    EXPECT_THROW(vehicle.driveKm(-1), InvalidVehicleDataException);
    vehicle.updateMileage(130000);
    EXPECT_EQ(vehicle.getMileage(), 130000);
    EXPECT_THROW(vehicle.updateMileage(1000), InvalidVehicleDataException);
}

TEST(VehicleTest, OwnershipTiresInsuranceAndWarranty) {
    Vehicle vehicle = makeVehicle();
    vehicle.assignOwnerId(7);
    EXPECT_EQ(vehicle.getOwnerId(), 7);
    EXPECT_EQ(vehicle.ageInYears(2026), 11);
    EXPECT_FALSE(vehicle.needsTireReplacement());
    vehicle.replaceTires(TireSet("215/60R16", "winter", 90.0, 2020));
    EXPECT_TRUE(vehicle.needsTireReplacement());
    const Insurance policy("P", "X", Date(2027, 1, 1), Money::fromUnits(1));
    EXPECT_TRUE(vehicle.isInsuredOn(policy, today()));
    EXPECT_FALSE(vehicle.isInsuredOn(policy, Date(2028, 1, 1)));
    const Warranty warranty("W", Date(2027, 1, 1), 200000);
    EXPECT_TRUE(vehicle.isUnderWarranty(warranty, today()));
}

TEST(ServicePlanTest, DueDetection) {
    Vehicle vehicle = makeVehicle();
    ServicePlan plan("Замена масла", 10000, 115000);
    EXPECT_EQ(plan.kmUntilDue(vehicle), 5000);
    EXPECT_FALSE(plan.isDue(vehicle));
    vehicle.driveKm(6000);
    EXPECT_TRUE(plan.isDue(vehicle));
    plan.recordService(vehicle);
    EXPECT_EQ(plan.getLastServiceKm(), 126000);
    EXPECT_FALSE(plan.isDue(vehicle));
    EXPECT_EQ(plan.getServiceName(), std::string("Замена масла"));
    EXPECT_EQ(plan.getIntervalKm(), 10000);
}

TEST(DiagnosticResultTest, SeverityAndPriority) {
    const DiagnosticResult critical("P0300", "Пропуски зажигания", 5, Money::fromUnits(300));
    EXPECT_TRUE(critical.isCritical());
    EXPECT_EQ(critical.priority(), 0);
    const DiagnosticResult minor("P0420", "Катализатор", 2, Money::fromUnits(100));
    EXPECT_FALSE(minor.isCritical());
    EXPECT_EQ(minor.priority(), 3);
    EXPECT_EQ(minor.getCode(), std::string("P0420"));
    EXPECT_EQ(minor.getDescription(), std::string("Катализатор"));
    EXPECT_EQ(minor.getSeverity(), 2);
    EXPECT_EQ(minor.getEstimatedCost(), Money::fromUnits(100));
}
