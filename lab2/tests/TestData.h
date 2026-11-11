#pragma once

#include <string>

#include "autoshop/AutoShop.h"

namespace testdata {

using namespace autoshop;

inline const char* const kVin = "WVWZZZ1JZXW000001";
inline const char* const kSecondVin = "WVWZZZ1JZXW000002";

inline Date today() { return Date(2026, 11, 10); }

inline Person makePerson(const std::string& name = "Анна Иванова", int birthYear = 1990) {
    return Person(name, Date(birthYear, 5, 20), ContactInfo("+375 29 123-45-67", "anna@example.com"),
                  Address("ул. Ленина 1", "Минск", "220000"));
}

inline Client makeClient(int id = 1) { return Client(id, makePerson(), Date(2026, 1, 15)); }

inline Vehicle makeVehicle(const std::string& vin = kVin) {
    return Vehicle(vin, "1234 AB-7", "Volkswagen", "Golf", 2015, 120000, Engine("petrol", 1.6, 105),
                   TireSet("205/55R16", "summer", 20.0, 2023));
}

inline Employee makeEmployee(int id = 10, const std::string& name = "Пётр Сидоров") {
    return Employee(id, makePerson(name, 1985), "Механик", Date(2020, 3, 1), Money::fromUnits(2000));
}

inline Mechanic makeMechanic(int id = 10) { return Mechanic(makeEmployee(id), "engine", Money::fromUnits(30)); }

inline ServiceType oilChange() { return ServiceType("OIL", "Замена масла", Money::fromUnits(20), 1.0, "engine"); }
inline ServiceType diagnostics() { return ServiceType("DIAG", "Диагностика", Money::fromUnits(10), 0.5, "electrics"); }
inline ServiceType engineOverhaul() {
    return ServiceType("ENGINE", "Капитальный ремонт двигателя", Money::fromUnits(1200), 10.0, "engine");
}

inline Part oilFilter() {
    return Part("F-001", "Масляный фильтр", "filters", Money::fromUnits(15), {"Volkswagen", "Audi"});
}
inline Part brakePad() { return Part("B-010", "Колодки", "brakes", Money::fromUnits(60)); }

inline Supplier makeSupplier() {
    return Supplier("АвтоЗапчасть", ContactInfo("+375 17 222-33-44", "sales@parts.by"), 3, 4.5);
}

inline Discount makeDiscount(double percent = 10.0) {
    return Discount(percent, "Акция", Date(2026, 11, 1), Date(2026, 11, 30));
}

/// Клиент с зарегистрированным автомобилем.
struct Garage {
    Garage() : vehicle(makeVehicle()), client(makeClient()) { client.registerVehicle(vehicle); }
    Vehicle vehicle;
    Client client;
};

/// Завершённый заказ-наряд на 1 час работы.
inline WorkOrder makeCompletedOrder(const Garage& garage, int orderId = 100) {
    WorkOrder order(orderId, garage.client, garage.vehicle, today());
    order.addService(oilChange());
    order.assignMechanic(makeMechanic());
    order.start();
    order.complete(1.0);
    return order;
}

inline AutoShop makeShop() {
    AutoShop shop("Автомастерская «Гараж»", Address("ул. Мира 5", "Минск", "220001"), 2, Money::fromUnits(30));
    shop.addServiceType(oilChange());
    shop.addServiceType(engineOverhaul());
    shop.addPart(oilFilter(), 10, 2);
    shop.addPart(brakePad(), 1, 2);
    return shop;
}

}  // namespace testdata
