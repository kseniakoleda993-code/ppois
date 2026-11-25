#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Grades.h"
#include "People.h"
#include "Structure.h"

namespace studis {

inline constexpr double kIntensiveHoursPerCredit = 40.0;
inline constexpr int kLatePenaltyPerDay = 10;
inline constexpr int kLateGraceDays = 3;
inline constexpr const char* kLectureKind = "lecture";
inline constexpr const char* kPracticeKind = "practice";
inline constexpr const char* kLabKind = "lab";

/// Учебная дисциплина.
class Course {
public:
    Course(std::string code, std::string title, int credits, int hours, std::string departmentName)
        : code_(std::move(code)), title_(std::move(title)), credits_(credits), hours_(hours),
          departmentName_(std::move(departmentName)) {
        if (code_.empty() || title_.empty() || credits_ <= 0 || hours_ < credits_) {
            throw InvalidCourseDataException(code_);
        }
    }

    const std::string& getCode() const { return code_; }
    const std::string& getTitle() const { return title_; }
    int getCredits() const { return credits_; }
    int getHours() const { return hours_; }
    const std::string& getDepartmentName() const { return departmentName_; }
    double hoursPerCredit() const { return static_cast<double>(hours_) / credits_; }
    bool isIntensive() const { return hoursPerCredit() > kIntensiveHoursPerCredit; }
    bool belongsTo(const Department& department) const { return departmentName_ == department.getName(); }

private:
    std::string code_;
    std::string title_;
    int credits_;
    int hours_;
    std::string departmentName_;
};

/// Пререквизит: чтобы изучать дисциплину, нужно сдать другую не ниже заданного балла.
class CoursePrerequisite {
public:
    CoursePrerequisite(std::string courseCode, std::string requiredCourseCode, int minPoints)
        : courseCode_(std::move(courseCode)), requiredCourseCode_(std::move(requiredCourseCode)),
          minPoints_(minPoints) {}

    const std::string& getCourseCode() const { return courseCode_; }
    const std::string& getRequiredCourseCode() const { return requiredCourseCode_; }
    int getMinPoints() const { return minPoints_; }
    bool isSatisfiedBy(const RecordBook& recordBook) const {
        return recordBook.hasPassed(requiredCourseCode_, minPoints_);
    }

private:
    std::string courseCode_;
    std::string requiredCourseCode_;
    int minPoints_;
};

/// Реализация дисциплины в семестре: преподаватель, места и записавшиеся студенты.
class CourseOffering {
public:
    CourseOffering(int id, const Course& course, const Semester& semester, const Teacher& teacher, int capacity)
        : id_(id), courseCode_(course.getCode()), credits_(course.getCredits()), semesterYear_(semester.getYear()),
          term_(semester.getTerm()), teacherId_(teacher.getEmployeeId()), capacity_(capacity) {
        if (capacity_ <= 0) {
            throw InvalidCourseDataException("вместимость курса " + courseCode_);
        }
    }

    int getId() const { return id_; }
    const std::string& getCourseCode() const { return courseCode_; }
    int getCredits() const { return credits_; }
    int getSemesterYear() const { return semesterYear_; }
    Term getTerm() const { return term_; }
    int getTeacherId() const { return teacherId_; }
    int getCapacity() const { return capacity_; }
    const std::vector<int>& getEnrolledIds() const { return enrolledIds_; }
    int seatsLeft() const { return capacity_ - static_cast<int>(enrolledIds_.size()); }
    bool isFull() const { return seatsLeft() <= 0; }
    bool isEnrolled(const Student& student) const {
        return std::find(enrolledIds_.begin(), enrolledIds_.end(), student.getId()) != enrolledIds_.end();
    }
    void addPrerequisite(const CoursePrerequisite& prerequisite) { prerequisites_.push_back(prerequisite); }
    void enroll(const Student& student, const RecordBook& recordBook) {
        if (isEnrolled(student)) {
            throw DuplicateRecordException(student.getName() + " уже записан на " + courseCode_);
        }
        if (student.isOnProbation()) {
            throw AcademicProbationException(student.getName());
        }
        if (!student.isActive()) {
            throw InvalidStudentDataException("студент не обучается: " + student.getName());
        }
        if (isFull()) {
            throw CourseFullException(courseCode_);
        }
        requirePrerequisites(recordBook);
        enrolledIds_.push_back(student.getId());
    }
    void drop(const Student& student) {
        if (!isEnrolled(student)) {
            throw NotFoundException("запись студента на " + courseCode_);
        }
        enrolledIds_.erase(std::remove(enrolledIds_.begin(), enrolledIds_.end(), student.getId()), enrolledIds_.end());
    }

private:
    void requirePrerequisites(const RecordBook& recordBook) const {
        for (const CoursePrerequisite& prerequisite : prerequisites_) {
            if (!prerequisite.isSatisfiedBy(recordBook)) {
                throw PrerequisiteNotMetException(prerequisite.getRequiredCourseCode());
            }
        }
    }

    int id_;
    std::string courseCode_;
    int credits_;
    int semesterYear_;
    Term term_;
    int teacherId_;
    int capacity_;
    std::vector<int> enrolledIds_;
    std::vector<CoursePrerequisite> prerequisites_;
};

/// Учебное занятие (лекция, практика, лабораторная).
class Lesson {
public:
    Lesson(int id, const CourseOffering& offering, const Classroom& classroom, const LessonTime& time, std::string kind)
        : id_(id), offeringId_(offering.getId()), teacherId_(offering.getTeacherId()),
          classroomLabel_(classroom.label()), time_(time), kind_(std::move(kind)) {
        if (kind_ != kLectureKind && kind_ != kPracticeKind && kind_ != kLabKind) {
            throw InvalidCourseDataException("вид занятия " + kind_);
        }
        if (!classroom.canFit(static_cast<int>(offering.getEnrolledIds().size()))) {
            throw RoomCapacityException("аудитория " + classroomLabel_);
        }
    }

    int getId() const { return id_; }
    int getOfferingId() const { return offeringId_; }
    int getTeacherId() const { return teacherId_; }
    const std::string& getClassroomLabel() const { return classroomLabel_; }
    const LessonTime& getTime() const { return time_; }
    const std::string& getKind() const { return kind_; }
    bool isLecture() const { return kind_ == kLectureKind; }
    int durationHours() const { return time_.getDurationHours(); }
    bool isOnDay(Weekday day) const { return time_.getDay() == day; }
    void reschedule(const LessonTime& newTime) { time_ = newTime; }
    bool conflictsWith(const Lesson& other) const {
        return time_.overlaps(other.time_) && (classroomLabel_ == other.classroomLabel_ || teacherId_ == other.teacherId_);
    }

private:
    int id_;
    int offeringId_;
    int teacherId_;
    std::string classroomLabel_;
    LessonTime time_;
    std::string kind_;
};

/// Расписание занятий на семестр.
class Timetable {
public:
    explicit Timetable(const Semester& semester) : semesterYear_(semester.getYear()) {}

    int getSemesterYear() const { return semesterYear_; }
    const std::vector<Lesson>& getLessons() const { return lessons_; }
    bool hasConflict(const Lesson& lesson) const {
        return std::any_of(lessons_.begin(), lessons_.end(), [&](const Lesson& l) { return l.conflictsWith(lesson); });
    }
    void addLesson(const Lesson& lesson) {
        if (hasConflict(lesson)) {
            throw ScheduleConflictException("занятие №" + std::to_string(lesson.getId()));
        }
        lessons_.push_back(lesson);
    }
    void removeLesson(int lessonId) {
        auto found = std::find_if(lessons_.begin(), lessons_.end(), [&](const Lesson& l) { return l.getId() == lessonId; });
        if (found == lessons_.end()) {
            throw NotFoundException("занятие " + std::to_string(lessonId));
        }
        lessons_.erase(found);
    }
    std::vector<Lesson> lessonsOn(Weekday day) const {
        std::vector<Lesson> result;
        std::copy_if(lessons_.begin(), lessons_.end(), std::back_inserter(result),
                     [&](const Lesson& l) { return l.isOnDay(day); });
        return result;
    }
    std::vector<Lesson> lessonsOfTeacher(int teacherId) const {
        std::vector<Lesson> result;
        std::copy_if(lessons_.begin(), lessons_.end(), std::back_inserter(result),
                     [&](const Lesson& l) { return l.getTeacherId() == teacherId; });
        return result;
    }
    int totalHoursFor(int offeringId) const {
        int total = 0;
        for (const Lesson& lesson : lessons_) {
            total += lesson.getOfferingId() == offeringId ? lesson.durationHours() : 0;
        }
        return total;
    }

private:
    int semesterYear_;
    std::vector<Lesson> lessons_;
};

/// Задание по дисциплине.
class Assignment {
public:
    Assignment(int id, const CourseOffering& offering, std::string title, const Date& dueDate, int maxScore)
        : id_(id), offeringId_(offering.getId()), title_(std::move(title)), dueDate_(dueDate), maxScore_(maxScore) {
        if (maxScore_ <= 0) {
            throw InvalidGradeException("максимальный балл задания");
        }
    }

    int getId() const { return id_; }
    int getOfferingId() const { return offeringId_; }
    const std::string& getTitle() const { return title_; }
    const Date& getDueDate() const { return dueDate_; }
    int getMaxScore() const { return maxScore_; }
    bool isOverdueOn(const Date& date) const { return dueDate_.isBefore(date); }
    int latePenaltyPercent(const Date& date) const {
        if (!isOverdueOn(date)) {
            return 0;
        }
        return std::min(static_cast<int>(kPercentBase), dueDate_.daysUntil(date) * kLatePenaltyPerDay);
    }
    bool acceptsSubmissionOn(const Date& date) const {
        return !isOverdueOn(date) || dueDate_.daysUntil(date) <= kLateGraceDays;
    }

private:
    int id_;
    int offeringId_;
    std::string title_;
    Date dueDate_;
    int maxScore_;
};

/// Работа студента по заданию.
class Submission {
public:
    Submission(const Assignment& assignment, const Student& student, const Date& submittedOn)
        : assignmentId_(assignment.getId()), studentId_(student.getId()), submittedOn_(submittedOn),
          penaltyPercent_(assignment.latePenaltyPercent(submittedOn)) {
        if (!assignment.acceptsSubmissionOn(submittedOn)) {
            throw DocumentException("срок сдачи задания истёк");
        }
    }

    int getAssignmentId() const { return assignmentId_; }
    int getStudentId() const { return studentId_; }
    const Date& getSubmittedOn() const { return submittedOn_; }
    int getScore() const { return score_; }
    int getPenaltyPercent() const { return penaltyPercent_; }
    bool isGraded() const { return score_ >= 0; }
    void grade(int rawScore, const Assignment& assignment) {
        if (rawScore < 0 || rawScore > assignment.getMaxScore()) {
            throw InvalidGradeException(std::to_string(rawScore));
        }
        score_ = static_cast<int>(rawScore * (kPercentBase - penaltyPercent_) / kPercentBase + 0.5);
    }
    double percentOfMax(const Assignment& assignment) const {
        return isGraded() ? kPercentBase * score_ / assignment.getMaxScore() : 0.0;
    }

private:
    int assignmentId_;
    int studentId_;
    Date submittedOn_;
    int score_ = -1;
    int penaltyPercent_;
};

}  // namespace studis
