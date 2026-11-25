#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Courses.h"
#include "Grades.h"
#include "People.h"
#include "Structure.h"

namespace studis {

enum class ExamStatus { Scheduled, InProgress, Finished };

/// Экзамен или зачёт по реализации дисциплины.
class Exam {
public:
    Exam(int id, const CourseOffering& offering, const Date& date, const Classroom& classroom)
        : id_(id), offeringId_(offering.getId()), date_(date), classroomLabel_(classroom.label()),
          roomCapacity_(classroom.getCapacity()) {}

    int getId() const { return id_; }
    int getOfferingId() const { return offeringId_; }
    const Date& getDate() const { return date_; }
    const std::string& getClassroomLabel() const { return classroomLabel_; }
    ExamStatus getStatus() const { return status_; }
    int attendeeCount() const { return static_cast<int>(registeredIds_.size()); }
    bool isRegistered(const Student& student) const {
        return std::find(registeredIds_.begin(), registeredIds_.end(), student.getId()) != registeredIds_.end();
    }
    void registerStudent(const Student& student) {
        if (status_ != ExamStatus::Scheduled) {
            throw InvalidExamStateException("регистрация после начала экзамена");
        }
        if (isRegistered(student)) {
            throw DuplicateRecordException(student.getName() + " уже записан на экзамен");
        }
        if (attendeeCount() >= roomCapacity_) {
            throw RoomCapacityException("аудитория " + classroomLabel_);
        }
        registeredIds_.push_back(student.getId());
    }
    void start() {
        if (status_ != ExamStatus::Scheduled) {
            throw InvalidExamStateException("экзамен уже начат");
        }
        status_ = ExamStatus::InProgress;
    }
    void finish() {
        if (status_ != ExamStatus::InProgress) {
            throw InvalidExamStateException("экзамен не идёт");
        }
        status_ = ExamStatus::Finished;
    }

private:
    int id_;
    int offeringId_;
    Date date_;
    std::string classroomLabel_;
    int roomCapacity_;
    ExamStatus status_ = ExamStatus::Scheduled;
    std::vector<int> registeredIds_;
};

/// Результат студента на экзамене.
class ExamResult {
public:
    ExamResult(const Exam& exam, const Student& student, const Grade& grade)
        : examId_(exam.getId()), studentId_(student.getId()), grade_(grade), takenOn_(exam.getDate()) {
        if (exam.getStatus() == ExamStatus::Scheduled) {
            throw InvalidExamStateException("экзамен ещё не начался");
        }
        if (!exam.isRegistered(student)) {
            throw NotFoundException("допуск студента " + student.getName() + " к экзамену");
        }
    }

    int getExamId() const { return examId_; }
    int getStudentId() const { return studentId_; }
    const Grade& getGrade() const { return grade_; }
    const Date& getTakenOn() const { return takenOn_; }
    bool isPassed() const { return grade_.isPassing(); }
    RecordEntry toRecordEntry(const Course& course, int attempt) const {
        return RecordEntry(course.getCode(), course.getCredits(), grade_, takenOn_, attempt);
    }
    void updateStanding(Student& student) const {
        if (isPassed()) {
            student.clearFailedExam();
        } else {
            student.registerFailedExam();
        }
    }

private:
    int examId_;
    int studentId_;
    Grade grade_;
    Date takenOn_;
};

enum class AttendanceStatus { Present, Absent, Excused, Late };

inline constexpr double kLateWeight = 0.5;
inline constexpr double kMinAttendancePercent = 60.0;

/// Отметка о посещении занятия.
class AttendanceRecord {
public:
    AttendanceRecord(const Lesson& lesson, const Student& student, AttendanceStatus status, const Date& date)
        : lessonId_(lesson.getId()), studentId_(student.getId()), status_(status), date_(date) {}

    int getLessonId() const { return lessonId_; }
    int getStudentId() const { return studentId_; }
    AttendanceStatus getStatus() const { return status_; }
    const Date& getDate() const { return date_; }
    bool countsAsPresent() const { return status_ == AttendanceStatus::Present || status_ == AttendanceStatus::Late; }
    bool isExcused() const { return status_ == AttendanceStatus::Excused; }
    double weight() const {
        if (status_ == AttendanceStatus::Present || status_ == AttendanceStatus::Excused) {
            return 1.0;
        }
        return status_ == AttendanceStatus::Late ? kLateWeight : 0.0;
    }

private:
    int lessonId_;
    int studentId_;
    AttendanceStatus status_;
    Date date_;
};

/// Журнал посещаемости реализации дисциплины.
class AttendanceJournal {
public:
    explicit AttendanceJournal(const CourseOffering& offering) : offeringId_(offering.getId()) {}

    int getOfferingId() const { return offeringId_; }
    const std::vector<AttendanceRecord>& getRecords() const { return records_; }
    void mark(const AttendanceRecord& record) {
        const bool duplicate = std::any_of(records_.begin(), records_.end(), [&](const AttendanceRecord& r) {
            return r.getLessonId() == record.getLessonId() && r.getStudentId() == record.getStudentId();
        });
        if (duplicate) {
            throw DuplicateRecordException("отметка студента " + std::to_string(record.getStudentId()));
        }
        records_.push_back(record);
    }
    double attendancePercent(const Student& student) const {
        double total = 0.0;
        int count = 0;
        for (const AttendanceRecord& record : records_) {
            if (record.getStudentId() == student.getId()) {
                total += record.weight();
                ++count;
            }
        }
        return count == 0 ? kPercentBase : kPercentBase * total / count;
    }
    int absencesOf(const Student& student) const {
        return static_cast<int>(std::count_if(records_.begin(), records_.end(), [&](const AttendanceRecord& r) {
            return r.getStudentId() == student.getId() && r.getStatus() == AttendanceStatus::Absent;
        }));
    }
    bool isAllowedToExam(const Student& student) const { return attendancePercent(student) >= kMinAttendancePercent; }
    std::vector<int> studentsBelow(double percent, const std::vector<Student>& students) const {
        std::vector<int> ids;
        for (const Student& student : students) {
            if (attendancePercent(student) < percent) {
                ids.push_back(student.getId());
            }
        }
        return ids;
    }

private:
    int offeringId_;
    std::vector<AttendanceRecord> records_;
};

inline constexpr int kMaxExamAttempts = 3;

/// Пересдача экзамена.
class Retake {
public:
    Retake(const ExamResult& failed, const Date& scheduledOn, int attempt)
        : studentId_(failed.getStudentId()), examId_(failed.getExamId()), attempt_(attempt), scheduledOn_(scheduledOn) {
        if (failed.isPassed()) {
            throw InvalidExamStateException("пересдача не требуется: экзамен сдан");
        }
        if (attempt_ < 2 || attempt_ > kMaxExamAttempts) {
            throw InvalidExamStateException("недопустимая попытка " + std::to_string(attempt_));
        }
    }

    int getStudentId() const { return studentId_; }
    int getExamId() const { return examId_; }
    int getAttempt() const { return attempt_; }
    const Date& getScheduledOn() const { return scheduledOn_; }
    bool isLastAttempt() const { return attempt_ == kMaxExamAttempts; }
    int nextAttempt() const { return attempt_ + 1; }
    void reschedule(const Date& newDate) {
        if (newDate.isBefore(scheduledOn_)) {
            throw InvalidDateException("пересдачу нельзя перенести назад");
        }
        scheduledOn_ = newDate;
    }
    int daysUntil(const Date& today) const { return today.daysUntil(scheduledOn_); }

private:
    int studentId_;
    int examId_;
    int attempt_;
    Date scheduledOn_;
};

enum class ThesisStatus { Draft, Submitted, Defended, Rejected };

/// Дипломная работа.
class Thesis {
public:
    Thesis(int id, const Student& student, const Professor& supervisor, std::string title, const Date& deadline)
        : id_(id), studentId_(student.getId()), supervisorId_(supervisor.getEmployeeId()), title_(std::move(title)),
          deadline_(deadline) {
        if (!supervisor.supervises(student)) {
            throw InvalidStudentDataException("профессор не руководит студентом " + student.getName());
        }
    }

    int getId() const { return id_; }
    int getStudentId() const { return studentId_; }
    int getSupervisorId() const { return supervisorId_; }
    const std::string& getTitle() const { return title_; }
    const Date& getDeadline() const { return deadline_; }
    ThesisStatus getStatus() const { return status_; }
    int getScore() const { return score_; }
    bool isLate(const Date& date) const { return deadline_.isBefore(date); }
    bool isDefended() const { return status_ == ThesisStatus::Defended; }
    void submit(const Date& date) {
        if (status_ != ThesisStatus::Draft) {
            throw InvalidExamStateException("работа уже сдана");
        }
        if (isLate(date)) {
            throw DocumentException("срок сдачи диплома истёк");
        }
        status_ = ThesisStatus::Submitted;
    }
    void defend(const Student& student, const Grade& grade) {
        if (status_ != ThesisStatus::Submitted) {
            throw InvalidExamStateException("защита без сданной работы");
        }
        if (student.isOnProbation()) {
            throw AcademicProbationException(student.getName());
        }
        score_ = grade.getPoints();
        status_ = grade.isPassing() ? ThesisStatus::Defended : ThesisStatus::Rejected;
    }
    void reject() { status_ = ThesisStatus::Rejected; }

private:
    int id_;
    int studentId_;
    int supervisorId_;
    std::string title_;
    Date deadline_;
    ThesisStatus status_ = ThesisStatus::Draft;
    int score_ = 0;
};

/// Диплом о высшем образовании.
class Diploma {
public:
    Diploma(const Student& student, const StudyProgram& program, const RecordBook& recordBook, const Thesis& thesis,
            const Date& issuedOn)
        : studentId_(student.getId()), programCode_(program.getCode()), issuedOn_(issuedOn),
          gpa_(recordBook.gpa()), withHonors_(recordBook.hasHonors()), thesisTitle_(thesis.getTitle()) {
        if (!thesis.isDefended()) {
            throw InvalidExamStateException("диплом не может быть выдан без защиты");
        }
        if (recordBook.earnedCredits() < program.getRequiredCredits()) {
            throw InsufficientCreditsException(student.getName());
        }
        number_ = programCode_ + "-" + std::to_string(issuedOn_.getYear()) + "-" + std::to_string(studentId_);
    }

    int getStudentId() const { return studentId_; }
    const std::string& getProgramCode() const { return programCode_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    double getGpa() const { return gpa_; }
    bool isWithHonors() const { return withHonors_; }
    const std::string& getNumber() const { return number_; }
    const std::string& getThesisTitle() const { return thesisTitle_; }
    bool isIssuedIn(int year) const { return issuedOn_.getYear() == year; }
    std::string honorsLabel() const { return withHonors_ ? "с отличием" : "без отличия"; }
    std::string describe() const { return "Диплом " + number_ + " (" + honorsLabel() + "), тема: " + thesisTitle_; }

private:
    int studentId_;
    std::string programCode_;
    Date issuedOn_;
    double gpa_;
    bool withHonors_;
    std::string thesisTitle_;
    std::string number_;
};

}  // namespace studis
