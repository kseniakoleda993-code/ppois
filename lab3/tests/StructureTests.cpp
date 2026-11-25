#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(BuildingTest, FloorsAndDescription) {
    const Building building = makeBuilding();
    EXPECT_TRUE(building.hasFloor(5));
    EXPECT_FALSE(building.hasFloor(6));
    EXPECT_FALSE(building.hasFloor(0));
    EXPECT_TRUE(building.isInCity("Минск"));
    EXPECT_NE(building.describe().find("Корпус 1"), std::string::npos);
    EXPECT_EQ(building.getName(), std::string("Корпус 1"));
    EXPECT_EQ(building.getFloors(), 5);
    EXPECT_TRUE(building.getAddress().isInCity("Минск"));
    EXPECT_THROW(Building("", Address(), 3), InvalidCourseDataException);
    EXPECT_THROW(Building("A", Address(), 0), InvalidCourseDataException);
}

TEST(DepartmentTest, TeachersAndHead) {
    const Teacher head = makeTeacher(100);
    Department department("Кафедра ПО", "ФКСиС", head);
    EXPECT_TRUE(department.isHeadedBy(head));
    EXPECT_EQ(department.teacherCount(), 1);
    const Teacher colleague = makeTeacher(103);
    department.addTeacher(colleague);
    EXPECT_TRUE(department.hasTeacher(colleague));
    EXPECT_THROW(department.addTeacher(colleague), DuplicateRecordException);
    EXPECT_THROW(department.addTeacher(makeTeacher(104, "Другая кафедра")), InvalidStudentDataException);
    EXPECT_EQ(department.getName(), std::string("Кафедра ПО"));
    EXPECT_EQ(department.getFacultyName(), std::string("ФКСиС"));
    EXPECT_EQ(department.getHeadTeacherId(), 100);
}

TEST(FacultyTest, DepartmentsAndSearch) {
    Faculty faculty("ФКСиС", makeDean());
    faculty.addDepartment(Department("Кафедра ПО", "ФКСиС", makeTeacher(100)));
    faculty.addDepartment(Department("Кафедра ИИ", "ФКСиС", makeTeacher(105, "Кафедра ИИ")));
    EXPECT_THROW(faculty.addDepartment(Department("Кафедра ПО", "ФКСиС", makeTeacher(106))), DuplicateRecordException);
    EXPECT_EQ(faculty.findDepartment("Кафедра ИИ").getHeadTeacherId(), 105);
    EXPECT_THROW(faculty.findDepartment("Нет"), NotFoundException);
    EXPECT_EQ(faculty.totalTeachers(), 2);
    EXPECT_EQ(faculty.getName(), std::string("ФКСиС"));
    EXPECT_EQ(faculty.getDeanId(), 102);
    EXPECT_EQ(faculty.getDepartments().size(), static_cast<std::size_t>(2));
}

TEST(StudyProgramTest, DerivedValues) {
    const StudyProgram program = makeProgram();
    EXPECT_EQ(program.totalSemesters(), 8);
    EXPECT_EQ(program.creditsPerSemester(), 30);
    EXPECT_TRUE(program.isBachelor());
    EXPECT_FALSE(StudyProgram("M", "Магистратура", 2, 120, "master").isBachelor());
    EXPECT_EQ(program.graduationYear(Date(2023, 9, 1)), 2027);
    EXPECT_EQ(program.getCode(), std::string("CS"));
    EXPECT_EQ(program.getName(), std::string("Информатика"));
    EXPECT_EQ(program.getDurationYears(), 4);
    EXPECT_EQ(program.getRequiredCredits(), 240);
    EXPECT_EQ(program.getLevel(), std::string("bachelor"));
    EXPECT_THROW(StudyProgram("", "n", 4, 240, "b"), InvalidCourseDataException);
    EXPECT_THROW(StudyProgram("c", "", 4, 240, "b"), InvalidCourseDataException);
    EXPECT_THROW(StudyProgram("c", "n", 0, 240, "b"), InvalidCourseDataException);
    EXPECT_THROW(StudyProgram("c", "n", 4, 0, "b"), InvalidCourseDataException);
}

TEST(SemesterTest, PeriodsAndValidation) {
    const Semester semester = makeSemester();
    EXPECT_TRUE(semester.isActiveOn(today()));
    EXPECT_FALSE(semester.isActiveOn(Date(2026, 8, 1)));
    EXPECT_FALSE(semester.isExamPeriodOn(today()));
    EXPECT_TRUE(semester.isExamPeriodOn(Date(2026, 12, 25)));
    EXPECT_EQ(semester.daysUntilExams(today()), 40);
    EXPECT_EQ(semester.daysUntilExams(Date(2027, 1, 1)), 0);
    EXPECT_EQ(semester.lengthInDays(), 141);
    EXPECT_EQ(semester.getYear(), 2026);
    EXPECT_TRUE(semester.getTerm() == Term::Autumn);
    EXPECT_TRUE(semester.getStartDate() == Date(2026, 9, 1));
    EXPECT_TRUE(semester.getExamStartDate() == Date(2026, 12, 20));
    EXPECT_TRUE(semester.getEndDate() == Date(2027, 1, 20));
    EXPECT_THROW(Semester(2026, Term::Spring, Date(2026, 9, 1), Date(2026, 8, 1), Date(2027, 1, 1)), InvalidDateException);
    EXPECT_THROW(Semester(2026, Term::Spring, Date(2026, 9, 1), Date(2026, 12, 1), Date(2026, 11, 1)), InvalidDateException);
}

TEST(GroupTest, AddingStudents) {
    Group group("CS-101", makeProgram(), 2023, 2);
    Student first = makeStudent(1);
    Student second = makeStudent(2);
    group.addStudent(first);
    EXPECT_EQ(first.getGroupCode(), std::string("CS-101"));
    EXPECT_TRUE(group.hasStudent(first));
    EXPECT_THROW(group.addStudent(first), DuplicateRecordException);
    Student foreign = makeStudent(3, "SE");
    EXPECT_THROW(group.addStudent(foreign), InvalidStudentDataException);
    group.addStudent(second);
    EXPECT_TRUE(group.isFull());
    Student fourth = makeStudent(4);
    EXPECT_THROW(group.addStudent(fourth), RoomCapacityException);
    group.removeStudent(second);
    EXPECT_EQ(group.size(), 1);
    EXPECT_EQ(group.getCode(), std::string("CS-101"));
    EXPECT_EQ(group.getProgramCode(), std::string("CS"));
    EXPECT_EQ(group.getAdmissionYear(), 2023);
    EXPECT_EQ(group.getMaxSize(), 2);
    EXPECT_EQ(group.getStudentIds().size(), static_cast<std::size_t>(1));
}

TEST(GroupTest, CuratorAndCourseNumber) {
    Group group("CS-101", makeProgram(), 2023, 25);
    group.assignCurator(makeTeacher(100));
    EXPECT_EQ(group.getCuratorId(), 100);
    EXPECT_EQ(group.courseNumberOn(Date(2026, 11, 10)), 4);
    EXPECT_EQ(group.courseNumberOn(Date(2026, 5, 10)), 3);
}

TEST(ClassroomTest, CapacityAndLabel) {
    const Classroom room = makeClassroom("101", 2);
    EXPECT_TRUE(room.canFit(2));
    EXPECT_FALSE(room.canFit(3));
    EXPECT_EQ(room.label(), std::string("Корпус 1-101"));
    Group group("CS-101", makeProgram(), 2023, 5);
    EXPECT_TRUE(room.isSuitableFor(group));
    EXPECT_TRUE(room.hasProjector());
    EXPECT_EQ(room.getBuildingName(), std::string("Корпус 1"));
    EXPECT_EQ(room.getNumber(), std::string("101"));
    EXPECT_EQ(room.getCapacity(), 2);
    EXPECT_THROW(Classroom(makeBuilding(), "", 10, false), InvalidCourseDataException);
    EXPECT_THROW(Classroom(makeBuilding(), "1", 0, false), InvalidCourseDataException);
}

TEST(CurriculumEntryTest, Properties) {
    const CurriculumEntry entry("MATH", 1, 6, true);
    EXPECT_TRUE(entry.isInSemester(1));
    EXPECT_FALSE(entry.isInSemester(2));
    EXPECT_FALSE(entry.isElective());
    EXPECT_TRUE(CurriculumEntry("ART", 2, 3, false).isElective());
    EXPECT_EQ(entry.getCourseCode(), std::string("MATH"));
    EXPECT_EQ(entry.getSemesterNumber(), 1);
    EXPECT_EQ(entry.getCredits(), 6);
    EXPECT_TRUE(entry.isMandatory());
}

TEST(CurriculumTest, EntriesAndCredits) {
    Curriculum curriculum(makeProgram());
    curriculum.addEntry(CurriculumEntry("MATH", 1, 6, true));
    curriculum.addEntry(CurriculumEntry("ART", 1, 3, false));
    curriculum.addEntry(CurriculumEntry("PHYS", 2, 5, true));
    EXPECT_EQ(curriculum.totalCredits(), 14);
    EXPECT_EQ(curriculum.creditsInSemester(1), 9);
    EXPECT_EQ(curriculum.mandatoryCourses().size(), static_cast<std::size_t>(2));
    EXPECT_FALSE(curriculum.coversProgram());
    EXPECT_TRUE(curriculum.hasCourse("ART"));
    EXPECT_EQ(curriculum.getProgramCode(), std::string("CS"));
    EXPECT_EQ(curriculum.getEntries().size(), static_cast<std::size_t>(3));
    EXPECT_THROW(curriculum.addEntry(CurriculumEntry("MATH", 3, 6, true)), InvalidCourseDataException);
    EXPECT_THROW(curriculum.addEntry(CurriculumEntry("X", 9, 6, true)), InvalidCourseDataException);
    EXPECT_THROW(curriculum.addEntry(CurriculumEntry("Y", 0, 6, true)), InvalidCourseDataException);
}

TEST(CurriculumTest, CoversProgramWhenEnoughCredits) {
    Curriculum curriculum(StudyProgram("P", "Проверка", 1, 10, "bachelor"));
    curriculum.addEntry(CurriculumEntry("A", 1, 10, true));
    EXPECT_TRUE(curriculum.coversProgram());
}
