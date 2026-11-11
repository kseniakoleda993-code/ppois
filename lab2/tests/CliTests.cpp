#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "AutoShopMenu.h"
#include "DemoData.h"
#include "Menu.h"

using namespace autoshop;

namespace {
const char* const kDemoVin = "WVWZZZ1JZXW000001";

bool contains(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

std::string runSession(const std::string& input) {
    AutoShop shop = makeDemoShop(Date(2026, 11, 10));
    std::istringstream in(input);
    std::ostringstream out;
    AutoShopMenu menu(shop, Date(2026, 11, 10), in, out);
    menu.run();
    return out.str();
}
}  // namespace

TEST(MenuTest, ExecutesChosenItemAndExits) {
    std::istringstream in("1\n1\n0\n");
    std::ostringstream out;
    Menu menu("Тест", in, out);
    int counter = 0;
    menu.addItem("Увеличить", [&counter] { ++counter; });
    menu.run();
    EXPECT_EQ(counter, 2);
    EXPECT_TRUE(contains(out.str(), "1. Увеличить"));
}

TEST(MenuTest, ReportsUnknownChoicesAndErrors) {
    std::istringstream in("abc\n7\n1\n0\n");
    std::ostringstream out;
    Menu menu("Тест", in, out, "Выход");
    menu.addItem("Падает", [] { throw std::runtime_error("сломалось"); });
    menu.run();
    EXPECT_TRUE(contains(out.str(), "Неизвестный пункт меню"));
    EXPECT_TRUE(contains(out.str(), "Ошибка: сломалось"));
    EXPECT_TRUE(contains(out.str(), "0. Выход"));
}

TEST(MenuTest, StopsAtEndOfInputAndPromptsFail) {
    std::istringstream empty("");
    std::ostringstream out;
    Menu menu("Тест", empty, out);
    menu.run();
    std::istringstream bad("not a number\n");
    EXPECT_THROW(promptValue<int>(bad, out, "? "), std::runtime_error);
    std::istringstream none("");
    EXPECT_THROW(promptLine(none, out, "? "), std::runtime_error);
}

TEST(AutoShopMenuTest, ToTextCoversAllStatuses) {
    EXPECT_EQ(toText(WorkOrderStatus::Opened), std::string("открыт"));
    EXPECT_EQ(toText(WorkOrderStatus::InProgress), std::string("в работе"));
    EXPECT_EQ(toText(WorkOrderStatus::Completed), std::string("выполнен"));
    EXPECT_EQ(toText(WorkOrderStatus::Cancelled), std::string("отменён"));
}

TEST(AutoShopMenuTest, DemoShopContent) {
    const AutoShop shop = makeDemoShop(Date(2026, 11, 10));
    EXPECT_EQ(shop.getClients().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(shop.getMechanics().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(shop.getCatalog().size(), static_cast<std::size_t>(3));
}

TEST(AutoShopMenuTest, FullRepairSession) {
    const std::string vin = kDemoVin;
    const std::string output = runSession(
        "1\n4\n5\n2\n" + vin + "\nOIL\n9\n6\n2\n" + vin + "\n7\n4\nOIL\n8\n4\nF-001\n1\n9\n4\n1\n10\n4\n1.0\n11\n4\n"
        "12\n5\n65\ncard\n13\n14\n0\n");
    EXPECT_TRUE(contains(output, "Механик #1 Пётр Сидоров"));
    EXPECT_TRUE(contains(output, "Замена масла"));
    EXPECT_TRUE(contains(output, "Запись оформлена, номер 3"));
    EXPECT_TRUE(contains(output, "Заказ-наряд открыт, номер 4"));
    EXPECT_TRUE(contains(output, "Работа начата"));
    EXPECT_TRUE(contains(output, "Счёт №5 на 65.00 BYN"));
    EXPECT_TRUE(contains(output, "Остаток по счёту: 0.00 BYN"));
    EXPECT_TRUE(contains(output, "Заказ №4 [выполнен]"));
    EXPECT_TRUE(contains(output, "Выручка: 65.00 BYN"));
}

TEST(AutoShopMenuTest, RegistrationSession) {
    const std::string output = runSession(
        "2\nИван Петров\n1992-04-15\n+375 29 765-43-21\nivan@mail.by\nул. Мира 3\nМинск\n220005\n"
        "3\n2\nWVWZZZ1JZXW000009\n5678 CD-1\nSkoda\nOctavia\n2018\n90000\n1\n0\n");
    EXPECT_TRUE(contains(output, "Клиент зарегистрирован, номер 3"));
    EXPECT_TRUE(contains(output, "Автомобиль добавлен"));
    EXPECT_TRUE(contains(output, "Skoda Octavia"));
}

TEST(AutoShopMenuTest, ErrorsAreShownWithoutExit) {
    const std::string output = runSession(
        "2\nИван\nbad-date\n6\n999\nX\n5\n2\n" + std::string(kDemoVin) + "\nOIL\nabc\n7\n1\nOIL\n3\n0\n");
    EXPECT_TRUE(contains(output, "Дата должна быть в формате"));
    EXPECT_TRUE(contains(output, "Ошибка: Не найдено"));
    EXPECT_TRUE(contains(output, "Ошибка: Некорректный ввод"));
}

TEST(AutoShopMenuTest, LowStockIsReported) {
    const std::string vin = kDemoVin;
    const std::string output = runSession(
        "6\n2\n" + vin + "\n8\n3\nB-010\n18\n4\n0\n");
    EXPECT_TRUE(contains(output, "Мало на складе: B-010"));
}
