/**
 * @file CliTests.cpp
 * @author Коледа Ксения
 * @brief Модульные тесты консольных меню (ввод и вывод подменяются строковыми потоками).
 */
#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "BigIntegerMenu.h"
#include "Menu.h"
#include "VectorMenu.h"

using lab1::BigIntegerMenu;
using lab1::Menu;
using lab1::VectorMenu;

namespace {
bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

std::string runVectorMenu(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    VectorMenu menu(in, out);
    menu.run();
    return out.str();
}

std::string runBigIntegerMenu(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    BigIntegerMenu menu(in, out);
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

TEST(MenuTest, ReportsUnknownChoices) {
    std::istringstream in("abc\n7\n-3\n0\n");
    std::ostringstream out;
    Menu menu("Тест", in, out);
    menu.run();
    EXPECT_TRUE(contains(out.str(), "Неизвестный пункт меню"));
}

TEST(MenuTest, CatchesExceptionsFromActions) {
    std::istringstream in("1\n0\n");
    std::ostringstream out;
    Menu menu("Тест", in, out);
    menu.addItem("Падает", [] { throw std::runtime_error("сломалось"); });
    menu.run();
    EXPECT_TRUE(contains(out.str(), "Ошибка: сломалось"));
}

TEST(MenuTest, StopsAtEndOfInput) {
    std::istringstream in("");
    std::ostringstream out;
    Menu menu("Тест", in, out, "Выход");
    menu.run();
    EXPECT_TRUE(contains(out.str(), "0. Выход"));
}

TEST(MenuTest, PromptValueRejectsBadInput) {
    std::istringstream in("not a number\n");
    std::ostringstream out;
    EXPECT_THROW(lab1::promptValue<int>(in, out, "? "), std::runtime_error);
}

TEST(VectorMenuTest, ArithmeticOnEnteredVectors) {
    const std::string output = runVectorMenu(
        "1\n[(0, 0, 0), (1, 2, 3)]\n2\n[(0, 0, 0), (1, 2, 0)]\n3\n4\n5\n6\n0\n");
    EXPECT_TRUE(contains(output, "A = [(0, 0, 0), (1, 2, 3)]"));
    EXPECT_TRUE(contains(output, "A + B = [(0, 0, 0), (2, 4, 3)]"));
    EXPECT_TRUE(contains(output, "A - B = [(0, 0, 0), (0, 0, 3)]"));
    EXPECT_TRUE(contains(output, "A * B = [(0, 0, 0), (-6, 3, 0)]"));
}

TEST(VectorMenuTest, NumberOperationsAndMetrics) {
    const std::string output = runVectorMenu(
        "1\n[(0, 0, 0), (3, 4, 0)]\n2\n[(0, 0, 0), (0, 5, 0)]\n7\n2\n8\n2\n9\n10\n11\n12\n0\n");
    EXPECT_TRUE(contains(output, "A * 2 = [(0, 0, 0), (6, 8, 0)]"));
    EXPECT_TRUE(contains(output, "A / 2 = [(0, 0, 0), (1.5, 2, 0)]"));
    EXPECT_TRUE(contains(output, "|A| = 5"));
    EXPECT_TRUE(contains(output, "cos(A, B) = 0.8"));
    EXPECT_TRUE(contains(output, "A == B: 0"));
}

TEST(VectorMenuTest, ErrorsAreReportedWithoutExit) {
    const std::string output = runVectorMenu("8\nabc\n8\n0\n0\n");
    EXPECT_TRUE(contains(output, "Ошибка: Деление на ноль"));
    EXPECT_TRUE(contains(output, "Ошибка: Некорректный ввод"));
}

TEST(BigIntegerMenuTest, ArithmeticOnEnteredNumbers) {
    const std::string output = runBigIntegerMenu(
        "1\n123456789012345678901234567890\n2\n987654321098765432109876543210\n3\n4\n5\n6\n7\n0\n");
    EXPECT_TRUE(contains(output, "A + B = 1111111110111111111011111111100"));
    EXPECT_TRUE(contains(output, "A - B = -864197532086419753208641975320"));
    EXPECT_TRUE(contains(output, "A * B = 121932631137021795226185032733622923332237463801111263526900"));
    EXPECT_TRUE(contains(output, "A / B = 0"));
}

TEST(BigIntegerMenuTest, IncrementDecrementAndComparisons) {
    const std::string output = runBigIntegerMenu("1\n5\n2\n7\n8\n9\n10\n0\n");
    EXPECT_TRUE(contains(output, "A++ вернул 5, теперь A = 6"));
    EXPECT_TRUE(contains(output, "++A вернул 7"));
    EXPECT_TRUE(contains(output, "A-- вернул 7, теперь A = 6"));
    EXPECT_TRUE(contains(output, "--A вернул 5"));
    EXPECT_TRUE(contains(output, "A < B: 1"));
}

TEST(BigIntegerMenuTest, IntegerOperationsConversionAndFactorial) {
    const std::string output = runBigIntegerMenu("1\n100\n11\n7\n12\n13\n25\n0\n");
    EXPECT_TRUE(contains(output, "A + 7 = 107"));
    EXPECT_TRUE(contains(output, "A / 7 = 14"));
    EXPECT_TRUE(contains(output, "A как long long: 100"));
    EXPECT_TRUE(contains(output, "25! = 15511210043330985984000000"));
}

TEST(BigIntegerMenuTest, ErrorsAreReportedWithoutExit) {
    const std::string output = runBigIntegerMenu("1\nabc\n7\n1\n99999999999999999999\n12\n0\n");
    EXPECT_TRUE(contains(output, "Ошибка: Некорректный ввод"));
    EXPECT_TRUE(contains(output, "Ошибка: Деление на ноль"));
    EXPECT_TRUE(contains(output, "Ошибка: Число не помещается в long long"));
}
