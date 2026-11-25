#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "DemoData.h"
#include "Menu.h"
#include "StudentSystemMenu.h"

using namespace studis;

namespace {
bool contains(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

std::string runSession(const std::string& input) {
    StudentInformationSystem system = makeDemoSystem(Date(2026, 11, 10));
    std::istringstream in(input);
    std::ostringstream out;
    StudentSystemMenu menu(system, Date(2026, 11, 10), in, out);
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

TEST(StudentSystemMenuTest, ToTextCoversAllStatuses) {
    EXPECT_EQ(toText(StudentStatus::Active), std::string("учится"));
    EXPECT_EQ(toText(StudentStatus::OnLeave), std::string("в академотпуске"));
    EXPECT_EQ(toText(StudentStatus::Expelled), std::string("отчислен"));
    EXPECT_EQ(toText(StudentStatus::Graduated), std::string("выпускник"));
}

TEST(StudentSystemMenuTest, DemoSystemContent) {
    const StudentInformationSystem system = makeDemoSystem(Date(2026, 11, 10));
    EXPECT_EQ(system.getStudents().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(system.getTeachers().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(system.getCourses().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(system.getDormitory().getRooms().size(), static_cast<std::size_t>(3));
}

TEST(StudentSystemMenuTest, AcademicSession) {
    const std::string output = runSession(
        "1\n4\n5\nMATH\n1\n3\n3\n2\nCS-101\n6\n2\n3\n7\n3\n101\n9\n8\n2\nMATH\n85\n9\n2\n10\n11\n2\n4\n0\n");
    EXPECT_TRUE(contains(output, "Преподаватель #1 Пётр Сидоров"));
    EXPECT_TRUE(contains(output, "Математический анализ"));
    EXPECT_TRUE(contains(output, "Курс открыт, номер 3"));
    EXPECT_TRUE(contains(output, "Студент записан в группу"));
    EXPECT_TRUE(contains(output, "Студент записан на курс"));
    EXPECT_TRUE(contains(output, "Занятие добавлено, номер 4"));
    EXPECT_TRUE(contains(output, "MATH: 85 (B)"));
    EXPECT_TRUE(contains(output, "Средний балл: 85"));
    EXPECT_TRUE(contains(output, "Должников: 0"));
    EXPECT_TRUE(contains(output, "Зачётных единиц пока недостаточно"));
    EXPECT_TRUE(contains(output, "Курс #3 MATH"));
}

TEST(StudentSystemMenuTest, MoneyAndDormitorySession) {
    const std::string output = runSession("12\n2\n1000\n13\n3\n400\nonline\n14\n2\n0\n");
    EXPECT_TRUE(contains(output, "Договор заключён, номер 3"));
    EXPECT_TRUE(contains(output, "Платёж принят"));
    EXPECT_TRUE(contains(output, "Студент заселён в комнату 1"));
}

TEST(StudentSystemMenuTest, AdmissionAndDebtors) {
    const std::string output = runSession(
        "2\nИван Петров\n2007-04-15\n+375 29 765-43-21\nCS\n8\n2\nMATH\n30\n10\n1\n0\n");
    EXPECT_TRUE(contains(output, "Студент принят, номер 3"));
    EXPECT_TRUE(contains(output, "Должников: 1"));
    EXPECT_TRUE(contains(output, "Студент #3 Иван Петров"));
}

TEST(StudentSystemMenuTest, ErrorsAreShownWithoutExit) {
    const std::string output = runSession(
        "2\nИван\nbad-date\n8\n2\nNOPE\n50\n8\n2\nMATH\n150\n6\nabc\n14\n999\n0\n");
    EXPECT_TRUE(contains(output, "Дата должна быть в формате"));
    EXPECT_TRUE(contains(output, "Ошибка: Не найдено"));
    EXPECT_TRUE(contains(output, "Недопустимая оценка"));
    EXPECT_TRUE(contains(output, "Ошибка: Некорректный ввод"));
}
