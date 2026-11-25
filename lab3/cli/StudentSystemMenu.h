#pragma once

#include <iosfwd>
#include <string>

#include "Menu.h"
#include "studis/StudentInformationSystem.h"

namespace studis {

/// Консольное меню информационной системы студентов. Логика остаётся в классах из include/studis.
class StudentSystemMenu {
public:
    /// @param system система, с которой работает меню
    /// @param today «сегодняшняя» дата (передаётся снаружи, чтобы меню было тестируемым)
    /// @param in поток ввода
    /// @param out поток вывода
    StudentSystemMenu(StudentInformationSystem& system, const Date& today, std::istream& in, std::ostream& out);

    /// Запускает главное меню.
    void run();

private:
    void buildMenu(Menu& menu);
    void buildAcademicMenu(Menu& menu);
    void buildMoneyMenu(Menu& menu);
    void showStudents();
    void admitStudent();
    void assignToGroup();
    void showCourses();
    void openOffering();
    void enrollStudent();
    void scheduleLesson();
    void recordGrade();
    void showRecordBook();
    void signContract();
    void payTuition();
    void settleInDormitory();
    void showDebtors();
    void checkGraduation();
    int askInt(const std::string& message);
    Date askDate(const std::string& message);

    StudentInformationSystem& system_;
    Date today_;
    std::istream& in_;
    std::ostream& out_;
};

/// @return текстовое название статуса студента
std::string toText(StudentStatus status);

}  // namespace studis
