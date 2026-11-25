#pragma once

#include <string>
#include <utility>

#include "Common.h"
#include "Grades.h"
#include "People.h"

namespace studis {

/// Справка для студента (с места учёбы и т. п.).
class Certificate {
public:
    Certificate(int number, const Student& student, std::string type, const Date& issuedOn, int validDays)
        : number_(number), studentId_(student.getId()), type_(std::move(type)), issuedOn_(issuedOn),
          validUntil_(issuedOn.plusDays(validDays)) {}

    int getNumber() const { return number_; }
    int getStudentId() const { return studentId_; }
    const std::string& getType() const { return type_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    const Date& getValidUntil() const { return validUntil_; }
    bool isValidOn(const Date& date) const { return !date.isBefore(issuedOn_) && !validUntil_.isBefore(date); }
    int daysLeft(const Date& date) const { return std::max(0, date.daysUntil(validUntil_)); }
    void verify(const Student& student, const Date& date) const {
        if (student.getId() != studentId_ || !isValidOn(date)) {
            throw DocumentException("справка №" + std::to_string(number_) + " недействительна");
        }
    }
    void extend(int days) { validUntil_ = validUntil_.plusDays(days); }

private:
    int number_;
    int studentId_;
    std::string type_;
    Date issuedOn_;
    Date validUntil_;
};

/// Студенческий билет с электронным кошельком.
class StudentCard {
public:
    StudentCard(const Student& student, std::string number, const Date& issuedOn, const Date& validUntil)
        : studentId_(student.getId()), number_(std::move(number)), issuedOn_(issuedOn), validUntil_(validUntil) {
        if (validUntil_.isBefore(issuedOn_)) {
            throw DocumentException("срок билета раньше даты выдачи");
        }
    }

    int getStudentId() const { return studentId_; }
    const std::string& getNumber() const { return number_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    const Date& getValidUntil() const { return validUntil_; }
    const Money& getBalance() const { return balance_; }
    bool isValidOn(const Date& date) const { return !date.isBefore(issuedOn_) && !validUntil_.isBefore(date); }
    void extend(int days) { validUntil_ = validUntil_.plusDays(days); }
    void topUp(const Money& amount) {
        if (amount.isNegative() || amount.isZero()) {
            throw InvalidMoneyOperationException("пополнение должно быть положительным");
        }
        balance_ = balance_.plus(amount);
    }
    void pay(const Money& amount, const Date& date) {
        if (!isValidOn(date)) {
            throw DocumentException("студенческий билет " + number_ + " просрочен");
        }
        if (!balance_.isGreaterOrEqual(amount)) {
            throw InvalidMoneyOperationException("недостаточно средств на карте");
        }
        balance_ = balance_.minus(amount);
    }

private:
    int studentId_;
    std::string number_;
    Date issuedOn_;
    Date validUntil_;
    Money balance_;
};

/// Производственная практика студента.
class Internship {
public:
    Internship(int id, const Student& student, std::string company, const Date& startDate, const Date& endDate,
               std::string supervisorName)
        : id_(id), studentId_(student.getId()), company_(std::move(company)), startDate_(startDate),
          endDate_(endDate), supervisorName_(std::move(supervisorName)) {
        if (endDate_.isBefore(startDate_)) {
            throw InvalidDateException("конец практики раньше начала");
        }
    }

    int getId() const { return id_; }
    int getStudentId() const { return studentId_; }
    const std::string& getCompany() const { return company_; }
    const Date& getStartDate() const { return startDate_; }
    const Date& getEndDate() const { return endDate_; }
    const std::string& getSupervisorName() const { return supervisorName_; }
    int getReportScore() const { return reportScore_; }
    int lengthInDays() const { return startDate_.daysUntil(endDate_); }
    bool isActiveOn(const Date& date) const { return !date.isBefore(startDate_) && !endDate_.isBefore(date); }
    bool isCompletedOn(const Date& date) const { return endDate_.isBefore(date); }
    bool hasReport() const { return reportScore_ >= 0; }
    void submitReport(const Grade& grade, const Date& date) {
        if (!isCompletedOn(date)) {
            throw InvalidExamStateException("практика ещё не завершена");
        }
        reportScore_ = grade.getPoints();
    }

private:
    int id_;
    int studentId_;
    std::string company_;
    Date startDate_;
    Date endDate_;
    std::string supervisorName_;
    int reportScore_ = -1;
};

}  // namespace studis
