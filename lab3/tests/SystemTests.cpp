#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
struct Fixture {
    Fixture() : system(makeSystem()) {
        studentId = system.admitStudent(makePerson(), "CS", Date(2023, 9, 1));
        teacherId = system.hireTeacher(makePerson("Пётр Сидоров", 1975), "Кафедра ПО", Date(2005, 9, 1), "phd");
    }
    StudentInformationSystem system;
    int studentId = 0;
    int teacherId = 0;
};
}  // namespace

TEST(SystemTest, ConstructionAndCatalogs) {
    const StudentInformationSystem system = makeSystem();
    EXPECT_EQ(system.getName(), std::string("Университет"));
    EXPECT_TRUE(system.getAddress().isInCity("Минск"));
    EXPECT_EQ(system.getSemester().getYear(), 2026);
    EXPECT_EQ(system.getPrograms().size(), static_cast<std::size_t>(3));
    EXPECT_EQ(system.getCourses().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(system.getClassrooms().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(system.getGroups().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(system.getDormitory().getRooms().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(system.getTimetable().getSemesterYear(), 2026);
    EXPECT_EQ(system.findProgram("CS").getCode(), std::string("CS"));
    EXPECT_EQ(system.findCourse("MATH").getCredits(), 6);
    EXPECT_EQ(system.findClassroom("101").getCapacity(), 30);
    EXPECT_EQ(system.findGroup("CS-101").getMaxSize(), 2);
    EXPECT_THROW(system.findProgram("XX"), NotFoundException);
    EXPECT_THROW(system.findCourse("XX"), NotFoundException);
    EXPECT_THROW(system.findClassroom("999"), NotFoundException);
    EXPECT_THROW(system.findGroup("XX"), NotFoundException);
}

TEST(SystemTest, AdmissionAndGroups) {
    Fixture f;
    EXPECT_EQ(f.system.getStudents().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.system.getTeachers().size(), static_cast<std::size_t>(1));
    EXPECT_THROW(f.system.admitStudent(makePerson(), "XX", today()), NotFoundException);
    f.system.assignToGroup(f.studentId, "CS-101");
    EXPECT_EQ(f.system.findStudent(f.studentId).getGroupCode(), std::string("CS-101"));
    const int second = f.system.admitStudent(makePerson("Второй", 2005), "CS", Date(2023, 9, 1));
    f.system.assignToGroup(second, "CS-101");
    const int third = f.system.admitStudent(makePerson("Третий", 2005), "CS", Date(2023, 9, 1));
    EXPECT_THROW(f.system.assignToGroup(third, "CS-101"), RoomCapacityException);
    EXPECT_THROW(f.system.assignToGroup(999, "CS-101"), NotFoundException);
    EXPECT_THROW(f.system.findStudent(999), NotFoundException);
    EXPECT_EQ(f.system.findTeacher(f.teacherId).getName(), std::string("Пётр Сидоров"));
    EXPECT_THROW(f.system.findTeacher(999), NotFoundException);
}

TEST(SystemTest, OfferingsEnrollmentAndWorkload) {
    Fixture f;
    const int offeringId = f.system.openOffering("MATH", f.teacherId, 1);
    EXPECT_EQ(f.system.findTeacher(f.teacherId).getWeeklyHours(), 2);
    f.system.enrollStudent(f.studentId, offeringId);
    EXPECT_EQ(f.system.findOffering(offeringId).getEnrolledIds().size(), static_cast<std::size_t>(1));
    const int other = f.system.admitStudent(makePerson("Второй", 2005), "CS", Date(2023, 9, 1));
    EXPECT_THROW(f.system.enrollStudent(other, offeringId), CourseFullException);
    EXPECT_THROW(f.system.enrollStudent(f.studentId, 999), NotFoundException);
    EXPECT_THROW(f.system.openOffering("MATH", 999, 5), NotFoundException);
    EXPECT_THROW(f.system.openOffering("NOPE", f.teacherId, 5), NotFoundException);
    EXPECT_EQ(f.system.getOfferings().size(), static_cast<std::size_t>(1));
    EXPECT_THROW(f.system.findOffering(999), NotFoundException);
}

TEST(SystemTest, TimetableRules) {
    Fixture f;
    const int first = f.system.openOffering("MATH", f.teacherId, 5);
    const int second = f.system.openOffering("PHYS", f.teacherId, 5);
    f.system.scheduleLesson(first, "101", LessonTime(Weekday::Monday, 9, 2), "lecture");
    EXPECT_THROW(f.system.scheduleLesson(second, "101", LessonTime(Weekday::Monday, 10, 2), "lecture"),
                 ScheduleConflictException);
    f.system.scheduleLesson(second, "101", LessonTime(Weekday::Monday, 11, 2), "practice");
    EXPECT_EQ(f.system.getTimetable().getLessons().size(), static_cast<std::size_t>(2));
    const int small = f.system.openOffering("MATH", f.teacherId, 5);
    const int other = f.system.admitStudent(makePerson("Второй", 2005), "CS", Date(2023, 9, 1));
    f.system.enrollStudent(f.studentId, small);
    f.system.enrollStudent(other, small);
    EXPECT_THROW(f.system.scheduleLesson(small, "102", LessonTime(Weekday::Friday, 9, 2), "lecture"),
                 RoomCapacityException);
}

TEST(SystemTest, GradesGpaAndDebtors) {
    Fixture f;
    f.system.recordGrade(f.studentId, "MATH", 90, today());
    f.system.recordGrade(f.studentId, "PHYS", 40, today());
    EXPECT_NEAR(f.system.gpaOf(f.studentId), 65.0, 1e-9);
    EXPECT_EQ(f.system.debtors().size(), static_cast<std::size_t>(1));
    EXPECT_EQ(f.system.findStudent(f.studentId).getFailedExams(), 1);
    f.system.recordGrade(f.studentId, "PHYS", 70, today());
    EXPECT_EQ(f.system.findStudent(f.studentId).getFailedExams(), 0);
    EXPECT_TRUE(f.system.debtors().empty());
    EXPECT_THROW(f.system.recordGrade(f.studentId, "NOPE", 50, today()), NotFoundException);
    EXPECT_THROW(f.system.recordGrade(f.studentId, "MATH", 150, today()), InvalidGradeException);
    EXPECT_THROW(f.system.gpaOf(999), NotFoundException);
    EXPECT_EQ(f.system.findRecordBook(f.studentId).getEntries().size(), static_cast<std::size_t>(2));
}

TEST(SystemTest, GraduationEligibility) {
    Fixture f;
    const int miniStudent = f.system.admitStudent(makePerson("Мини", 2005), "MINI", Date(2025, 9, 1));
    EXPECT_FALSE(f.system.canGraduate(miniStudent));
    f.system.recordGrade(miniStudent, "MATH", 75, today());
    EXPECT_TRUE(f.system.canGraduate(miniStudent));
    EXPECT_FALSE(f.system.canGraduate(f.studentId));
}

TEST(SystemTest, TuitionContractsAndDormitory) {
    Fixture f;
    const int contractId = f.system.signContract(f.studentId, Money::fromUnits(1000), 4);
    f.system.payTuition(contractId, Money::fromUnits(400), today(), "online");
    EXPECT_EQ(f.system.getContracts().front().outstanding(), Money::fromUnits(600));
    EXPECT_THROW(f.system.payTuition(contractId, Money::fromUnits(9999), today(), "online"), TuitionPaymentException);
    EXPECT_THROW(f.system.payTuition(999, Money(1), today(), "online"), NotFoundException);
    EXPECT_EQ(f.system.settleInDormitory(f.studentId), 1);
    const int other = f.system.admitStudent(makePerson("Второй", 2005), "CS", Date(2023, 9, 1));
    EXPECT_THROW(f.system.settleInDormitory(other), RoomCapacityException);
}
