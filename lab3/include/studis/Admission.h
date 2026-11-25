#pragma once

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Courses.h"
#include "Grades.h"
#include "People.h"
#include "Structure.h"

namespace studis {

inline constexpr int kMaxLeaveDays = 365;

/// Абитуриент.
class Applicant : public Person {
public:
    Applicant(int id, const Person& person, std::vector<int> examScores, double schoolAverage)
        : Person(person), id_(id), examScores_(std::move(examScores)), schoolAverage_(schoolAverage) {
        if (examScores_.empty()) {
            throw AdmissionException("нет результатов экзаменов у " + getName());
        }
        for (int score : examScores_) {
            if (score < 0 || score > kMaxPoints) {
                throw InvalidGradeException(std::to_string(score));
            }
        }
    }

    int getId() const { return id_; }
    const std::vector<int>& getExamScores() const { return examScores_; }
    double getSchoolAverage() const { return schoolAverage_; }
    int totalScore() const { return std::accumulate(examScores_.begin(), examScores_.end(), 0); }
    int bestScore() const { return *std::max_element(examScores_.begin(), examScores_.end()); }
    double averageScore() const { return static_cast<double>(totalScore()) / static_cast<double>(examScores_.size()); }
    bool meetsMinimum(int minTotal) const { return totalScore() >= minTotal; }

private:
    int id_;
    std::vector<int> examScores_;
    double schoolAverage_;
};

/// Приёмная кампания на направление.
class AdmissionCampaign {
public:
    AdmissionCampaign(int year, const StudyProgram& program, int quota, int minScore, const Date& deadline)
        : year_(year), programCode_(program.getCode()), quota_(quota), minScore_(minScore), deadline_(deadline) {
        if (quota_ <= 0) {
            throw AdmissionException("квота должна быть положительной");
        }
    }

    int getYear() const { return year_; }
    const std::string& getProgramCode() const { return programCode_; }
    int getQuota() const { return quota_; }
    int getMinScore() const { return minScore_; }
    const Date& getDeadline() const { return deadline_; }
    const std::vector<int>& getAcceptedIds() const { return acceptedIds_; }
    bool isOpenOn(const Date& date) const { return !deadline_.isBefore(date); }
    int remainingPlaces() const { return quota_ - static_cast<int>(acceptedIds_.size()); }
    bool isAccepted(const Applicant& applicant) const {
        return std::find(acceptedIds_.begin(), acceptedIds_.end(), applicant.getId()) != acceptedIds_.end();
    }
    double fillPercent() const { return kPercentBase * acceptedIds_.size() / quota_; }
    void accept(const Applicant& applicant, const Date& date) {
        if (!isOpenOn(date)) {
            throw AdmissionException("приём документов закрыт " + deadline_.toString());
        }
        if (!applicant.meetsMinimum(minScore_)) {
            throw AdmissionException("проходной балл не достигнут: " + applicant.getName());
        }
        if (isAccepted(applicant)) {
            throw DuplicateRecordException("абитуриент уже зачислен: " + applicant.getName());
        }
        if (remainingPlaces() <= 0) {
            throw AdmissionException("места закончились");
        }
        acceptedIds_.push_back(applicant.getId());
    }

private:
    int year_;
    std::string programCode_;
    int quota_;
    int minScore_;
    Date deadline_;
    std::vector<int> acceptedIds_;
};

enum class ApplicationStatus { Pending, Accepted, Rejected };

/// Заявление абитуриента на направление.
class Application {
public:
    Application(int id, const Applicant& applicant, const StudyProgram& program, int priority, const Date& submittedOn)
        : id_(id), applicantId_(applicant.getId()), programCode_(program.getCode()), priority_(priority),
          submittedOn_(submittedOn) {
        if (priority_ < 1) {
            throw AdmissionException("приоритет должен быть не меньше 1");
        }
    }

    int getId() const { return id_; }
    int getApplicantId() const { return applicantId_; }
    const std::string& getProgramCode() const { return programCode_; }
    int getPriority() const { return priority_; }
    const Date& getSubmittedOn() const { return submittedOn_; }
    ApplicationStatus getStatus() const { return status_; }
    bool isHigherPriorityThan(const Application& other) const { return priority_ < other.priority_; }
    void approve() {
        requirePending();
        status_ = ApplicationStatus::Accepted;
    }
    void reject() {
        requirePending();
        status_ = ApplicationStatus::Rejected;
    }
    bool evaluate(AdmissionCampaign& campaign, const Applicant& applicant, const Date& date) {
        try {
            campaign.accept(applicant, date);
        } catch (const AdmissionException&) {
            reject();
            return false;
        }
        approve();
        return true;
    }

private:
    void requirePending() const {
        if (status_ != ApplicationStatus::Pending) {
            throw AdmissionException("заявление уже рассмотрено");
        }
    }

    int id_;
    int applicantId_;
    std::string programCode_;
    int priority_;
    Date submittedOn_;
    ApplicationStatus status_ = ApplicationStatus::Pending;
};

enum class EnrollmentStatus { Active, Dropped, Completed };

/// Запись студента на реализацию дисциплины.
class Enrollment {
public:
    Enrollment(const Student& student, const CourseOffering& offering, const Date& enrolledOn)
        : studentId_(student.getId()), offeringId_(offering.getId()), enrolledOn_(enrolledOn) {}

    int getStudentId() const { return studentId_; }
    int getOfferingId() const { return offeringId_; }
    const Date& getEnrolledOn() const { return enrolledOn_; }
    EnrollmentStatus getStatus() const { return status_; }
    bool isActive() const { return status_ == EnrollmentStatus::Active; }
    int daysEnrolled(const Date& date) const { return enrolledOn_.daysUntil(date); }
    void drop() {
        requireActive();
        status_ = EnrollmentStatus::Dropped;
    }
    void complete() {
        requireActive();
        status_ = EnrollmentStatus::Completed;
    }

private:
    void requireActive() const {
        if (!isActive()) {
            throw InvalidExamStateException("запись на дисциплину уже закрыта");
        }
    }

    int studentId_;
    int offeringId_;
    Date enrolledOn_;
    EnrollmentStatus status_ = EnrollmentStatus::Active;
};

/// Академический отпуск.
class AcademicLeave {
public:
    AcademicLeave(const Student& student, const Date& startDate, const Date& endDate, std::string reason)
        : studentId_(student.getId()), startDate_(startDate), endDate_(endDate), reason_(std::move(reason)) {
        if (endDate_.isBefore(startDate_) || lengthInDays() > kMaxLeaveDays) {
            throw InvalidDateException("недопустимый срок отпуска");
        }
        if (!student.isActive()) {
            throw InvalidStudentDataException("отпуск доступен только обучающимся: " + student.getName());
        }
    }

    int getStudentId() const { return studentId_; }
    const Date& getStartDate() const { return startDate_; }
    const Date& getEndDate() const { return endDate_; }
    const std::string& getReason() const { return reason_; }
    int getApprovedById() const { return approvedById_; }
    int lengthInDays() const { return startDate_.daysUntil(endDate_); }
    bool isActiveOn(const Date& date) const { return !date.isBefore(startDate_) && !endDate_.isBefore(date); }
    bool isApproved() const { return approvedById_ != 0; }
    void extend(int days) {
        if (lengthInDays() + days > kMaxLeaveDays) {
            throw InvalidDateException("отпуск не может быть дольше года");
        }
        endDate_ = endDate_.plusDays(days);
    }
    void approve(const Dean& dean) { approvedById_ = dean.getEmployeeId(); }

private:
    int studentId_;
    Date startDate_;
    Date endDate_;
    std::string reason_;
    int approvedById_ = 0;
};

enum class RequestStatus { Pending, Approved, Rejected, Applied };

/// Заявление на перевод на другое направление.
class TransferRequest {
public:
    TransferRequest(int id, const Student& student, const StudyProgram& target, std::string reason)
        : id_(id), studentId_(student.getId()), fromProgramCode_(student.getProgramCode()),
          toProgramCode_(target.getCode()), reason_(std::move(reason)) {
        if (fromProgramCode_ == toProgramCode_) {
            throw InvalidStudentDataException("перевод на то же направление");
        }
    }

    int getId() const { return id_; }
    int getStudentId() const { return studentId_; }
    const std::string& getFromProgramCode() const { return fromProgramCode_; }
    const std::string& getToProgramCode() const { return toProgramCode_; }
    const std::string& getReason() const { return reason_; }
    RequestStatus getStatus() const { return status_; }
    int getApprovedById() const { return approvedById_; }
    bool isPending() const { return status_ == RequestStatus::Pending; }
    void approve(const Dean& dean) {
        requirePending();
        approvedById_ = dean.getEmployeeId();
        status_ = RequestStatus::Approved;
    }
    void reject() {
        requirePending();
        status_ = RequestStatus::Rejected;
    }
    void applyTo(Student& student) {
        if (status_ != RequestStatus::Approved) {
            throw AdmissionException("перевод не одобрен");
        }
        student.changeProgram(toProgramCode_);
        status_ = RequestStatus::Applied;
    }

private:
    void requirePending() const {
        if (!isPending()) {
            throw AdmissionException("заявление на перевод уже рассмотрено");
        }
    }

    int id_;
    int studentId_;
    std::string fromProgramCode_;
    std::string toProgramCode_;
    std::string reason_;
    RequestStatus status_ = RequestStatus::Pending;
    int approvedById_ = 0;
};

/// Приказ по студентам.
class AdministrativeOrder {
public:
    AdministrativeOrder(int number, std::string type, const Date& issuedOn, std::string text)
        : number_(number), type_(std::move(type)), issuedOn_(issuedOn), text_(std::move(text)) {}

    int getNumber() const { return number_; }
    const std::string& getType() const { return type_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    const std::string& getText() const { return text_; }
    int getSignedById() const { return signedById_; }
    bool isSigned() const { return signedById_ != 0; }
    void sign(const Dean& dean) {
        if (isSigned()) {
            throw DocumentException("приказ №" + std::to_string(number_) + " уже подписан");
        }
        signedById_ = dean.getEmployeeId();
    }
    bool isRecentOn(const Date& date, int days) const { return issuedOn_.daysUntil(date) <= days; }
    std::string describe() const { return "Приказ №" + std::to_string(number_) + " (" + type_ + ") от " + issuedOn_.toString(); }

private:
    int number_;
    std::string type_;
    Date issuedOn_;
    std::string text_;
    int signedById_ = 0;
};

}  // namespace studis
