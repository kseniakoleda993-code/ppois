#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Admission.h"
#include "Assessment.h"
#include "Common.h"
#include "Courses.h"
#include "Finance.h"
#include "Grades.h"
#include "Life.h"
#include "People.h"
#include "Structure.h"

namespace studis {

inline constexpr int kWeeklyHoursPerOffering = 2;
inline constexpr int kDormitoryFeeUnits = 25;
inline constexpr const char* kDormitoryName = "Общежитие №1";

/// Информационная система университета: объединяет студентов, преподавателей, дисциплины, расписание и финансы.
class StudentInformationSystem {
private:
    template <typename Container, typename Predicate>
    static auto& locateIn(Container& items, Predicate matches, const std::string& description) {
        auto found = std::find_if(items.begin(), items.end(), matches);
        if (found == items.end()) {
            throw NotFoundException(description);
        }
        return *found;
    }

public:
    StudentInformationSystem(std::string name, const Address& address, const Semester& currentSemester)
        : name_(std::move(name)), address_(address), semester_(currentSemester), timetable_(currentSemester),
          dormitory_(kDormitoryName, address, Money::fromUnits(kDormitoryFeeUnits)) {}

    const std::string& getName() const { return name_; }
    const Address& getAddress() const { return address_; }
    const Semester& getSemester() const { return semester_; }
    const std::vector<Student>& getStudents() const { return students_; }
    const std::vector<Teacher>& getTeachers() const { return teachers_; }
    const std::vector<StudyProgram>& getPrograms() const { return programs_; }
    const std::vector<Group>& getGroups() const { return groups_; }
    const std::vector<Course>& getCourses() const { return courses_; }
    const std::vector<CourseOffering>& getOfferings() const { return offerings_; }
    const std::vector<Classroom>& getClassrooms() const { return classrooms_; }
    const std::vector<TuitionContract>& getContracts() const { return contracts_; }
    const Timetable& getTimetable() const { return timetable_; }
    const Dormitory& getDormitory() const { return dormitory_; }

    void addProgram(const StudyProgram& program) { programs_.push_back(program); }
    void addCourse(const Course& course) { courses_.push_back(course); }
    void addClassroom(const Classroom& classroom) { classrooms_.push_back(classroom); }
    void addDormitoryRoom(int number, int capacity) { dormitory_.addRoom(DormitoryRoom(number, capacity)); }

    int admitStudent(const Person& person, const std::string& programCode, const Date& admissionDate) {
        findProgram(programCode);
        const int id = nextId();
        students_.emplace_back(id, person, programCode, admissionDate);
        recordBooks_.emplace_back(students_.back());
        return id;
    }
    int hireTeacher(const Person& person, const std::string& department, const Date& hiredOn, const std::string& degree) {
        const int id = nextId();
        teachers_.emplace_back(id, person, department, hiredOn, degree);
        return id;
    }
    void createGroup(const std::string& code, const std::string& programCode, int admissionYear, int maxSize) {
        groups_.emplace_back(code, findProgram(programCode), admissionYear, maxSize);
    }
    void assignToGroup(int studentId, const std::string& groupCode) {
        locateGroup(groupCode).addStudent(locateStudent(studentId));
    }
    int openOffering(const std::string& courseCode, int teacherId, int capacity) {
        Teacher& teacher = locateTeacher(teacherId);
        const CourseOffering offering(nextId(), findCourse(courseCode), semester_, teacher, capacity);
        teacher.addWorkload(kWeeklyHoursPerOffering);
        offerings_.push_back(offering);
        return offering.getId();
    }
    void enrollStudent(int studentId, int offeringId) {
        locateOffering(offeringId).enroll(findStudent(studentId), locateRecordBook(studentId));
    }
    int scheduleLesson(int offeringId, const std::string& classroomNumber, const LessonTime& time, const std::string& kind) {
        const Lesson lesson(nextId(), findOffering(offeringId), findClassroom(classroomNumber), time, kind);
        timetable_.addLesson(lesson);
        return lesson.getId();
    }
    void recordGrade(int studentId, const std::string& courseCode, int points, const Date& date) {
        const Course& course = findCourse(courseCode);
        const Grade grade(points);
        locateRecordBook(studentId).addEntry(RecordEntry(course.getCode(), course.getCredits(), grade, date, 1));
        Student& student = locateStudent(studentId);
        if (grade.isPassing()) {
            student.clearFailedExam();
        } else {
            student.registerFailedExam();
        }
    }
    double gpaOf(int studentId) const { return findRecordBook(studentId).gpa(); }
    bool canGraduate(int studentId) const {
        const Student& student = findStudent(studentId);
        const StudyProgram& program = findProgram(student.getProgramCode());
        return findRecordBook(studentId).earnedCredits() >= program.getRequiredCredits();
    }
    std::vector<int> debtors() const {
        std::vector<int> ids;
        for (const RecordBook& recordBook : recordBooks_) {
            if (!recordBook.failedCourses().empty()) {
                ids.push_back(recordBook.getStudentId());
            }
        }
        return ids;
    }
    int signContract(int studentId, const Money& fee, int installments) {
        const Student& student = findStudent(studentId);
        contracts_.emplace_back(nextId(), student, findProgram(student.getProgramCode()), fee, installments);
        return contracts_.back().getId();
    }
    void payTuition(int contractId, const Money& amount, const Date& paidOn, const std::string& method) {
        TuitionContract& contract = locateContract(contractId);
        const TuitionPayment payment(nextId(), contract, amount, paidOn, method);
        contract.registerPayment(payment.getAmount());
    }
    int settleInDormitory(int studentId) { return dormitory_.settle(findStudent(studentId)); }

    const Student& findStudent(int id) const {
        return locateIn(students_, [id](const Student& s) { return s.getId() == id; }, "студент " + std::to_string(id));
    }
    const Teacher& findTeacher(int id) const {
        return locateIn(teachers_, [id](const Teacher& t) { return t.getEmployeeId() == id; },
                        "преподаватель " + std::to_string(id));
    }
    const StudyProgram& findProgram(const std::string& code) const {
        return locateIn(programs_, [&code](const StudyProgram& p) { return p.getCode() == code; }, "направление " + code);
    }
    const Course& findCourse(const std::string& code) const {
        return locateIn(courses_, [&code](const Course& c) { return c.getCode() == code; }, "дисциплина " + code);
    }
    const CourseOffering& findOffering(int id) const {
        return locateIn(offerings_, [id](const CourseOffering& o) { return o.getId() == id; },
                        "курс " + std::to_string(id));
    }
    const Group& findGroup(const std::string& code) const {
        return locateIn(groups_, [&code](const Group& g) { return g.getCode() == code; }, "группа " + code);
    }
    const Classroom& findClassroom(const std::string& number) const {
        return locateIn(classrooms_, [&number](const Classroom& c) { return c.getNumber() == number; },
                        "аудитория " + number);
    }
    const RecordBook& findRecordBook(int studentId) const {
        return locateIn(recordBooks_, [studentId](const RecordBook& r) { return r.getStudentId() == studentId; },
                        "зачётная книжка студента " + std::to_string(studentId));
    }

private:
    int nextId() { return nextId_++; }
    Student& locateStudent(int id) {
        return locateIn(students_, [id](const Student& s) { return s.getId() == id; }, "студент " + std::to_string(id));
    }
    Teacher& locateTeacher(int id) {
        return locateIn(teachers_, [id](const Teacher& t) { return t.getEmployeeId() == id; },
                        "преподаватель " + std::to_string(id));
    }
    Group& locateGroup(const std::string& code) {
        return locateIn(groups_, [&code](const Group& g) { return g.getCode() == code; }, "группа " + code);
    }
    CourseOffering& locateOffering(int id) {
        return locateIn(offerings_, [id](const CourseOffering& o) { return o.getId() == id; },
                        "курс " + std::to_string(id));
    }
    RecordBook& locateRecordBook(int studentId) {
        return locateIn(recordBooks_, [studentId](const RecordBook& r) { return r.getStudentId() == studentId; },
                        "зачётная книжка студента " + std::to_string(studentId));
    }
    TuitionContract& locateContract(int id) {
        return locateIn(contracts_, [id](const TuitionContract& c) { return c.getId() == id; },
                        "договор " + std::to_string(id));
    }

    std::string name_;
    Address address_;
    Semester semester_;
    int nextId_ = 1;
    std::vector<Student> students_;
    std::vector<Teacher> teachers_;
    std::vector<StudyProgram> programs_;
    std::vector<Group> groups_;
    std::vector<Course> courses_;
    std::vector<CourseOffering> offerings_;
    std::vector<Classroom> classrooms_;
    std::vector<RecordBook> recordBooks_;
    std::vector<TuitionContract> contracts_;
    Timetable timetable_;
    Dormitory dormitory_;
};

}  // namespace studis
