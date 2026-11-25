#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "People.h"

namespace studis {

inline constexpr int kSemestersPerYear = 2;
inline constexpr int kAcademicYearStartMonth = 9;
inline constexpr const char* kBachelorLevel = "bachelor";

/// Учебный корпус.
class Building {
public:
    Building(std::string name, const Address& address, int floors)
        : name_(std::move(name)), address_(address), floors_(floors) {
        if (name_.empty() || floors_ <= 0) {
            throw InvalidCourseDataException("корпус " + name_);
        }
    }

    const std::string& getName() const { return name_; }
    const Address& getAddress() const { return address_; }
    int getFloors() const { return floors_; }
    bool hasFloor(int floor) const { return floor >= 1 && floor <= floors_; }
    bool isInCity(const std::string& city) const { return address_.isInCity(city); }
    std::string describe() const { return name_ + " (" + std::to_string(floors_) + " эт.), " + address_.fullAddress(); }

private:
    std::string name_;
    Address address_;
    int floors_;
};

/// Кафедра.
class Department {
public:
    Department(std::string name, std::string facultyName, const Teacher& head)
        : name_(std::move(name)), facultyName_(std::move(facultyName)), headTeacherId_(head.getEmployeeId()),
          teacherIds_{head.getEmployeeId()} {}

    const std::string& getName() const { return name_; }
    const std::string& getFacultyName() const { return facultyName_; }
    int getHeadTeacherId() const { return headTeacherId_; }
    bool hasTeacher(const Teacher& teacher) const {
        return std::find(teacherIds_.begin(), teacherIds_.end(), teacher.getEmployeeId()) != teacherIds_.end();
    }
    void addTeacher(const Teacher& teacher) {
        if (teacher.getDepartment() != name_) {
            throw InvalidStudentDataException("преподаватель из другой кафедры: " + teacher.getName());
        }
        if (hasTeacher(teacher)) {
            throw DuplicateRecordException("преподаватель уже на кафедре: " + teacher.getName());
        }
        teacherIds_.push_back(teacher.getEmployeeId());
    }
    int teacherCount() const { return static_cast<int>(teacherIds_.size()); }
    bool isHeadedBy(const Teacher& teacher) const { return teacher.getEmployeeId() == headTeacherId_; }

private:
    std::string name_;
    std::string facultyName_;
    int headTeacherId_;
    std::vector<int> teacherIds_;
};

/// Факультет.
class Faculty {
public:
    Faculty(std::string name, const Dean& dean) : name_(std::move(name)), deanId_(dean.getEmployeeId()) {}

    const std::string& getName() const { return name_; }
    int getDeanId() const { return deanId_; }
    const std::vector<Department>& getDepartments() const { return departments_; }
    void addDepartment(const Department& department) {
        if (std::any_of(departments_.begin(), departments_.end(),
                        [&](const Department& d) { return d.getName() == department.getName(); })) {
            throw DuplicateRecordException("кафедра " + department.getName());
        }
        departments_.push_back(department);
    }
    const Department& findDepartment(const std::string& name) const {
        for (const Department& department : departments_) {
            if (department.getName() == name) {
                return department;
            }
        }
        throw NotFoundException("кафедра " + name);
    }
    int totalTeachers() const {
        int total = 0;
        for (const Department& department : departments_) {
            total += department.teacherCount();
        }
        return total;
    }

private:
    std::string name_;
    int deanId_;
    std::vector<Department> departments_;
};

/// Направление подготовки.
class StudyProgram {
public:
    StudyProgram(std::string code, std::string name, int durationYears, int requiredCredits, std::string level)
        : code_(std::move(code)), name_(std::move(name)), durationYears_(durationYears),
          requiredCredits_(requiredCredits), level_(std::move(level)) {
        if (code_.empty() || name_.empty() || durationYears_ <= 0 || requiredCredits_ <= 0) {
            throw InvalidCourseDataException("направление " + code_);
        }
    }

    const std::string& getCode() const { return code_; }
    const std::string& getName() const { return name_; }
    int getDurationYears() const { return durationYears_; }
    int getRequiredCredits() const { return requiredCredits_; }
    const std::string& getLevel() const { return level_; }
    int totalSemesters() const { return durationYears_ * kSemestersPerYear; }
    int creditsPerSemester() const { return requiredCredits_ / totalSemesters(); }
    bool isBachelor() const { return level_ == kBachelorLevel; }
    int graduationYear(const Date& admissionDate) const { return admissionDate.getYear() + durationYears_; }

private:
    std::string code_;
    std::string name_;
    int durationYears_;
    int requiredCredits_;
    std::string level_;
};

enum class Term { Autumn, Spring };

/// Учебный семестр.
class Semester {
public:
    Semester(int year, Term term, const Date& startDate, const Date& examStartDate, const Date& endDate)
        : year_(year), term_(term), startDate_(startDate), examStartDate_(examStartDate), endDate_(endDate) {
        if (examStartDate_.isBefore(startDate_) || endDate_.isBefore(examStartDate_)) {
            throw InvalidDateException("нарушен порядок дат семестра");
        }
    }

    int getYear() const { return year_; }
    Term getTerm() const { return term_; }
    const Date& getStartDate() const { return startDate_; }
    const Date& getExamStartDate() const { return examStartDate_; }
    const Date& getEndDate() const { return endDate_; }
    bool isActiveOn(const Date& date) const { return !date.isBefore(startDate_) && !endDate_.isBefore(date); }
    bool isExamPeriodOn(const Date& date) const { return !date.isBefore(examStartDate_) && !endDate_.isBefore(date); }
    int lengthInDays() const { return startDate_.daysUntil(endDate_); }
    int daysUntilExams(const Date& date) const { return std::max(0, date.daysUntil(examStartDate_)); }

private:
    int year_;
    Term term_;
    Date startDate_;
    Date examStartDate_;
    Date endDate_;
};

/// Учебная группа.
class Group {
public:
    Group(std::string code, const StudyProgram& program, int admissionYear, int maxSize)
        : code_(std::move(code)), programCode_(program.getCode()), admissionYear_(admissionYear), maxSize_(maxSize) {}

    const std::string& getCode() const { return code_; }
    const std::string& getProgramCode() const { return programCode_; }
    int getAdmissionYear() const { return admissionYear_; }
    int getMaxSize() const { return maxSize_; }
    const std::vector<int>& getStudentIds() const { return studentIds_; }
    int getCuratorId() const { return curatorId_; }
    int size() const { return static_cast<int>(studentIds_.size()); }
    bool isFull() const { return size() >= maxSize_; }
    bool hasStudent(const Student& student) const {
        return std::find(studentIds_.begin(), studentIds_.end(), student.getId()) != studentIds_.end();
    }
    void addStudent(Student& student) {
        if (student.getProgramCode() != programCode_) {
            throw InvalidStudentDataException("студент другого направления: " + student.getName());
        }
        if (hasStudent(student)) {
            throw DuplicateRecordException("студент уже в группе " + code_);
        }
        if (isFull()) {
            throw RoomCapacityException("группа " + code_ + " заполнена");
        }
        student.assignGroup(code_);
        studentIds_.push_back(student.getId());
    }
    void removeStudent(const Student& student) {
        studentIds_.erase(std::remove(studentIds_.begin(), studentIds_.end(), student.getId()), studentIds_.end());
    }
    void assignCurator(const Teacher& teacher) { curatorId_ = teacher.getEmployeeId(); }
    int courseNumberOn(const Date& date) const {
        return date.getYear() - admissionYear_ + (date.getMonth() >= kAcademicYearStartMonth ? 1 : 0);
    }

private:
    std::string code_;
    std::string programCode_;
    int admissionYear_;
    int maxSize_;
    std::vector<int> studentIds_;
    int curatorId_ = 0;
};

/// Аудитория.
class Classroom {
public:
    Classroom(const Building& building, std::string number, int capacity, bool hasProjector)
        : buildingName_(building.getName()), number_(std::move(number)), capacity_(capacity),
          hasProjector_(hasProjector) {
        if (number_.empty() || capacity_ <= 0) {
            throw InvalidCourseDataException("аудитория " + number_);
        }
    }

    const std::string& getBuildingName() const { return buildingName_; }
    const std::string& getNumber() const { return number_; }
    int getCapacity() const { return capacity_; }
    bool hasProjector() const { return hasProjector_; }
    bool canFit(int students) const { return students <= capacity_; }
    bool isSuitableFor(const Group& group) const { return canFit(group.size()); }
    std::string label() const { return buildingName_ + "-" + number_; }

private:
    std::string buildingName_;
    std::string number_;
    int capacity_;
    bool hasProjector_;
};

/// Строка учебного плана: дисциплина в определённом семестре.
class CurriculumEntry {
public:
    CurriculumEntry(std::string courseCode, int semesterNumber, int credits, bool mandatory)
        : courseCode_(std::move(courseCode)), semesterNumber_(semesterNumber), credits_(credits),
          mandatory_(mandatory) {}

    const std::string& getCourseCode() const { return courseCode_; }
    int getSemesterNumber() const { return semesterNumber_; }
    int getCredits() const { return credits_; }
    bool isMandatory() const { return mandatory_; }
    bool isInSemester(int semester) const { return semesterNumber_ == semester; }
    bool isElective() const { return !mandatory_; }

private:
    std::string courseCode_;
    int semesterNumber_;
    int credits_;
    bool mandatory_;
};

/// Учебный план направления.
class Curriculum {
public:
    explicit Curriculum(const StudyProgram& program)
        : programCode_(program.getCode()), requiredCredits_(program.getRequiredCredits()),
          totalSemesters_(program.totalSemesters()) {}

    const std::string& getProgramCode() const { return programCode_; }
    const std::vector<CurriculumEntry>& getEntries() const { return entries_; }
    bool hasCourse(const std::string& courseCode) const {
        return std::any_of(entries_.begin(), entries_.end(),
                           [&](const CurriculumEntry& e) { return e.getCourseCode() == courseCode; });
    }
    void addEntry(const CurriculumEntry& entry) {
        if (hasCourse(entry.getCourseCode())) {
            throw InvalidCourseDataException("дисциплина уже в плане: " + entry.getCourseCode());
        }
        if (entry.getSemesterNumber() < 1 || entry.getSemesterNumber() > totalSemesters_) {
            throw InvalidCourseDataException("семестр вне плана: " + std::to_string(entry.getSemesterNumber()));
        }
        entries_.push_back(entry);
    }
    int totalCredits() const {
        int total = 0;
        for (const CurriculumEntry& entry : entries_) {
            total += entry.getCredits();
        }
        return total;
    }
    int creditsInSemester(int semester) const {
        int total = 0;
        for (const CurriculumEntry& entry : entries_) {
            total += entry.isInSemester(semester) ? entry.getCredits() : 0;
        }
        return total;
    }
    std::vector<std::string> mandatoryCourses() const {
        std::vector<std::string> codes;
        for (const CurriculumEntry& entry : entries_) {
            if (entry.isMandatory()) {
                codes.push_back(entry.getCourseCode());
            }
        }
        return codes;
    }
    bool coversProgram() const { return totalCredits() >= requiredCredits_; }

private:
    std::string programCode_;
    int requiredCredits_;
    int totalSemesters_;
    std::vector<CurriculumEntry> entries_;
};

}  // namespace studis
