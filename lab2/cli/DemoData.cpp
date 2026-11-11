#include "DemoData.h"

namespace autoshop {

namespace {
constexpr int kDemoBays = 2;
constexpr long long kDemoHourlyRate = 30;
constexpr int kDemoStock = 20;
constexpr int kDemoMinStock = 3;
}  // namespace

AutoShop makeDemoShop(const Date& today) {
    AutoShop shop("Автомастерская «Гараж»", Address("ул. Мира 5", "Минск", "220001"), kDemoBays,
                  Money::fromUnits(kDemoHourlyRate));
    shop.addServiceType(ServiceType("OIL", "Замена масла", Money::fromUnits(20), 1.0, "engine"));
    shop.addServiceType(ServiceType("BRAKE", "Замена колодок", Money::fromUnits(40), 1.5, "chassis"));
    shop.addServiceType(ServiceType("DIAG", "Компьютерная диагностика", Money::fromUnits(15), 0.5, "electrics"));
    shop.addPart(Part("F-001", "Масляный фильтр", "filters", Money::fromUnits(15)), kDemoStock, kDemoMinStock);
    shop.addPart(Part("B-010", "Тормозные колодки", "brakes", Money::fromUnits(60)), kDemoStock, kDemoMinStock);
    const Person mechanic("Пётр Сидоров", Date(1985, 3, 12), ContactInfo("+375 29 111-22-33", "petr@garage.by"),
                          Address("ул. Лесная 7", "Минск", "220002"));
    shop.hireMechanic(mechanic, "engine", Money::fromUnits(kDemoHourlyRate), today);
    const Person owner("Анна Иванова", Date(1990, 5, 20), ContactInfo("+375 29 123-45-67", "anna@example.com"),
                       Address("ул. Ленина 1", "Минск", "220000"));
    const int clientId = shop.registerClient(owner, today);
    shop.registerVehicle(clientId, Vehicle("WVWZZZ1JZXW000001", "1234 AB-7", "Volkswagen", "Golf", 2015, 120000,
                                           Engine("petrol", 1.6, 105), TireSet("205/55R16", "summer", 20.0, 2023)));
    return shop;
}

}  // namespace autoshop
