#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"

namespace studis {

inline constexpr int kMinStudentAge = 16;
inline constexpr int kProbationLimit = 3;
inline constexpr int kMaxWeeklyTeachingHours = 36;
inline constexpr int kMaxSupervisedStudents = 8;
inline constexpr int kSeniorTeacherYears = 10;
inline constexpr const char* kDoctorDegree = "doctor";

/// Человек: общие данные студентов, преподавателей и сотрудников.
class Person {
public:
    Person(std::string name, const Date& birthDate, const ContactInfo& contact, const Address& address)
        : name_(std::move(name)), birthDate_(birthDate), contact_(contact), address_(address) {}
    virtual ~Person() = default;

    const std::string& getName() const { return name_; }
    const Date& getBirthDate() const { return birthDate_; }
    const ContactInfo& getContact() const { return contact_; }
    const Address& getAddress() const { return address_; }
    int ageOn(const Date& date) const {
        const bool birthdayPassed = std::make_pair(date.getMonth(), date.getDay()) >=
                                    std::make_pair(birthDate_.getMonth(), birthDate_.getDay());
        return date.getYear() - birthDate_.getYear() - (birthdayPassed ? 0 : 1);
    }
    void moveTo(const Address& newAddress) { address_ = newAddress; }
    void updateContact(const ContactInfo& newContact) { contact_ = newContact; }

private:
    std::string name_;
    Date birthDate_;
    ContactInfo contact_;
    Address address_;
};

enum class StudentStatus { Active, OnLeave, Expelled, Graduated };

/// Студент вуза.
class Student : public Person {
public:
    Student(int id, const Person& person, std::string programCode, const Date& admissionDate)
        : Person(person), id_(id), programCode_(std::move(programCode)), admissionDate_(admissionDate) {
        if (getName().empty() || programCode_.empty()) {
            throw InvalidStudentDataException(getName());
        }
        if (ageOn(admissionDate_) < kMinStudentAge) {
            throw InvalidStudentDataException("слишком юный абитуриент " + getName());
        }
    }

    int getId() const { return id_; }
    const std::string& getProgramCode() const { return programCode_; }
    const std::string& getGroupCode() const { return groupCode_; }
    const Date& getAdmissionDate() const { return admissionDate_; }
    StudentStatus getStatus() const { return status_; }
    int getFailedExams() const { return failedExams_; }
    void assignGroup(const std::string& groupCode) {
        requireActive("назначение группы");
        groupCode_ = groupCode;
    }
    void changeProgram(const std::string& newProgramCode) {
        requireActive("смена направления");
        programCode_ = newProgramCode;
        groupCode_.clear();
    }
    void goOnLeave() {
        requireActive("академический отпуск");
        status_ = StudentStatus::OnLeave;
    }
    void returnFromLeave() {
        if (status_ != StudentStatus::OnLeave) {
            throw InvalidStudentDataException("студент не находится в отпуске");
        }
        status_ = StudentStatus::Active;
    }
    void expel() {
        if (status_ == StudentStatus::Graduated) {
            throw InvalidStudentDataException("выпускника нельзя отчислить");
        }
        status_ = StudentStatus::Expelled;
    }
    void graduate() {
        requireActive("выпуск");
        status_ = StudentStatus::Graduated;
    }
    void registerFailedExam() { ++failedExams_; }
    void clearFailedExam() { failedExams_ = std::max(0, failedExams_ - 1); }
    bool isOnProbation() const { return failedExams_ >= kProbationLimit; }
    bool isActive() const { return status_ == StudentStatus::Active; }
    int yearOfStudyOn(const Date& date) const { return admissionDate_.daysUntil(date) / kDaysPerYear + 1; }

private:
    void requireActive(const std::string& action) const {
        if (status_ != StudentStatus::Active) {
            throw InvalidStudentDataException(action + ": студент не обучается");
        }
    }

    int id_;
    std::string programCode_;
    std::string groupCode_;
    Date admissionDate_;
    StudentStatus status_ = StudentStatus::Active;
    int failedExams_ = 0;
};

/// Преподаватель.
class Teacher : public Person {
public:
    Teacher(int employeeId, const Person& person, std::string department, const Date& hiredOn, std::string degree)
        : Person(person), employeeId_(employeeId), department_(std::move(department)), hiredOn_(hiredOn),
          degree_(std::move(degree)) {}

    int getEmployeeId() const { return employeeId_; }
    const std::string& getDepartment() const { return department_; }
    const Date& getHiredOn() const { return hiredOn_; }
    const std::string& getDegree() const { return degree_; }
    int getWeeklyHours() const { return weeklyHours_; }
    void addWorkload(int hours) {
        if (hours <= 0 || weeklyHours_ + hours > kMaxWeeklyTeachingHours) {
            throw ScheduleConflictException("перегрузка преподавателя " + getName());
        }
        weeklyHours_ += hours;
    }
    void releaseWorkload(int hours) { weeklyHours_ = std::max(0, weeklyHours_ - hours); }
    int freeHours() const { return kMaxWeeklyTeachingHours - weeklyHours_; }
    int yearsOfServiceOn(const Date& date) const { return hiredOn_.daysUntil(date) / kDaysPerYear; }
    bool isSeniorOn(const Date& date) const { return yearsOfServiceOn(date) >= kSeniorTeacherYears; }
    bool hasDoctorateDegree() const { return degree_ == kDoctorDegree; }

private:
    int employeeId_;
    std::string department_;
    Date hiredOn_;
    std::string degree_;
    int weeklyHours_ = 0;
};

/// Профессор: руководит дипломниками и ведёт научную работу.
class Professor : public Teacher {
public:
    Professor(const Teacher& teacher, const Money& grantFunds) : Teacher(teacher), grantFunds_(grantFunds) {}

    int getPublications() const { return publications_; }
    const std::vector<int>& getSupervisedIds() const { return supervisedIds_; }
    const Money& getGrantFunds() const { return grantFunds_; }
    void publishPaper() { ++publications_; }
    bool canSupervise() const { return supervisedIds_.size() < static_cast<std::size_t>(kMaxSupervisedStudents); }
    bool supervises(const Student& student) const {
        return std::find(supervisedIds_.begin(), supervisedIds_.end(), student.getId()) != supervisedIds_.end();
    }
    void supervise(const Student& student) {
        if (supervises(student)) {
            throw DuplicateRecordException("студент уже под руководством " + getName());
        }
        if (!canSupervise()) {
            throw ScheduleConflictException("слишком много дипломников у " + getName());
        }
        supervisedIds_.push_back(student.getId());
    }
    void receiveGrant(const Money& amount) { grantFunds_ = grantFunds_.plus(amount); }
    void spendGrant(const Money& amount) {
        if (!grantFunds_.isGreaterOrEqual(amount)) {
            throw InvalidMoneyOperationException("недостаточно средств гранта");
        }
        grantFunds_ = grantFunds_.minus(amount);
    }

private:
    int publications_ = 0;
    std::vector<int> supervisedIds_;
    Money grantFunds_;
};

/// Декан факультета.
class Dean : public Teacher {
public:
    Dean(const Teacher& teacher, std::string facultyName) : Teacher(teacher), facultyName_(std::move(facultyName)) {}

    const std::string& getFacultyName() const { return facultyName_; }
    int getSignedDocuments() const { return signedDocuments_; }
    void signDocument() { ++signedDocuments_; }
    bool canApproveLeaveFor(const Student& student) const { return student.isActive(); }
    void reviewExpulsion(Student& student) {
        if (!student.isOnProbation()) {
            throw AcademicProbationException("нет оснований для отчисления " + student.getName());
        }
        student.expel();
        signDocument();
    }

private:
    std::string facultyName_;
    int signedDocuments_ = 0;
};

/// Сотрудник деканата (методист).
class Methodist : public Person {
public:
    Methodist(int employeeId, const Person& person, std::string office)
        : Person(person), employeeId_(employeeId), office_(std::move(office)) {}

    int getEmployeeId() const { return employeeId_; }
    const std::string& getOffice() const { return office_; }
    int getProcessedDocuments() const { return processedDocuments_; }
    bool verifyStudentData(const Student& student) const {
        return !student.getGroupCode().empty() && !student.getProgramCode().empty() && student.isActive();
    }
    void registerDocument() { ++processedDocuments_; }
    bool canIssueCertificateTo(const Student& student) const {
        return student.getStatus() == StudentStatus::Active || student.getStatus() == StudentStatus::OnLeave;
    }

private:
    int employeeId_;
    std::string office_;
    int processedDocuments_ = 0;
};

}  // namespace studis
