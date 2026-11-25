#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(PersonTest, AgeAndUpdates) {
    Person person = makePerson("Иван", 2000);
    EXPECT_EQ(person.ageOn(Date(2026, 5, 20)), 26);
    EXPECT_EQ(person.ageOn(Date(2026, 5, 19)), 25);
    person.moveTo(Address("ул. Новая 1", "Гомель", "246000"));
    person.updateContact(ContactInfo("+375 33 111-22-33", "new@mail.by"));
    EXPECT_TRUE(person.getAddress().isInCity("Гомель"));
    EXPECT_EQ(person.getContact().getEmail(), std::string("new@mail.by"));
    EXPECT_EQ(person.getName(), std::string("Иван"));
    EXPECT_TRUE(person.getBirthDate() == Date(2000, 5, 20));
}

TEST(StudentTest, CreationRules) {
    const Student student = makeStudent(5);
    EXPECT_EQ(student.getId(), 5);
    EXPECT_EQ(student.getProgramCode(), std::string("CS"));
    EXPECT_TRUE(student.getAdmissionDate() == Date(2023, 9, 1));
    EXPECT_TRUE(student.isActive());
    EXPECT_TRUE(student.getGroupCode().empty());
    EXPECT_THROW(Student(1, makePerson(""), "CS", today()), InvalidStudentDataException);
    EXPECT_THROW(Student(1, makePerson(), "", today()), InvalidStudentDataException);
    EXPECT_THROW(Student(1, makePerson("Малыш", 2015), "CS", today()), InvalidStudentDataException);
}

TEST(StudentTest, StatusTransitions) {
    Student student = makeStudent();
    student.assignGroup("CS-101");
    EXPECT_EQ(student.getGroupCode(), std::string("CS-101"));
    student.goOnLeave();
    EXPECT_EQ(student.getStatus(), StudentStatus::OnLeave);
    EXPECT_THROW(student.goOnLeave(), InvalidStudentDataException);
    EXPECT_THROW(student.assignGroup("X"), InvalidStudentDataException);
    student.returnFromLeave();
    EXPECT_TRUE(student.isActive());
    EXPECT_THROW(student.returnFromLeave(), InvalidStudentDataException);
}

TEST(StudentTest, GraduationAndExpulsion) {
    Student graduate = makeStudent(1);
    graduate.graduate();
    EXPECT_EQ(graduate.getStatus(), StudentStatus::Graduated);
    EXPECT_THROW(graduate.expel(), InvalidStudentDataException);
    EXPECT_THROW(graduate.graduate(), InvalidStudentDataException);
    Student other = makeStudent(2);
    other.expel();
    EXPECT_EQ(other.getStatus(), StudentStatus::Expelled);
}

TEST(StudentTest, ProbationAndProgramChange) {
    Student student = makeStudent();
    EXPECT_FALSE(student.isOnProbation());
    student.registerFailedExam();
    student.registerFailedExam();
    student.registerFailedExam();
    EXPECT_TRUE(student.isOnProbation());
    student.clearFailedExam();
    EXPECT_FALSE(student.isOnProbation());
    EXPECT_EQ(student.getFailedExams(), 2);
    student.assignGroup("CS-101");
    student.changeProgram("SE");
    EXPECT_EQ(student.getProgramCode(), std::string("SE"));
    EXPECT_TRUE(student.getGroupCode().empty());
    EXPECT_EQ(student.yearOfStudyOn(Date(2026, 11, 10)), 4);
    student.clearFailedExam();
    student.clearFailedExam();
    student.clearFailedExam();
    EXPECT_EQ(student.getFailedExams(), 0);
}

TEST(TeacherTest, WorkloadAndSeniority) {
    Teacher teacher = makeTeacher();
    teacher.addWorkload(30);
    EXPECT_EQ(teacher.freeHours(), 6);
    EXPECT_THROW(teacher.addWorkload(10), ScheduleConflictException);
    EXPECT_THROW(teacher.addWorkload(0), ScheduleConflictException);
    teacher.releaseWorkload(100);
    EXPECT_EQ(teacher.getWeeklyHours(), 0);
    EXPECT_TRUE(teacher.isSeniorOn(today()));
    EXPECT_FALSE(teacher.isSeniorOn(Date(2010, 1, 1)));
    EXPECT_EQ(teacher.yearsOfServiceOn(today()), 21);
    EXPECT_FALSE(teacher.hasDoctorateDegree());
    EXPECT_TRUE(makeProfessor().hasDoctorateDegree());
    EXPECT_EQ(teacher.getEmployeeId(), 100);
    EXPECT_EQ(teacher.getDepartment(), std::string("Кафедра ПО"));
    EXPECT_EQ(teacher.getDegree(), std::string("phd"));
    EXPECT_TRUE(teacher.getHiredOn() == Date(2005, 9, 1));
}

TEST(ProfessorTest, SupervisionLimits) {
    Professor professor = makeProfessor();
    const Student student = makeStudent(1);
    EXPECT_TRUE(professor.canSupervise());
    professor.supervise(student);
    EXPECT_TRUE(professor.supervises(student));
    EXPECT_THROW(professor.supervise(student), DuplicateRecordException);
    for (int id = 2; id <= 8; ++id) {
        professor.supervise(makeStudent(id));
    }
    EXPECT_FALSE(professor.canSupervise());
    EXPECT_THROW(professor.supervise(makeStudent(9)), ScheduleConflictException);
    EXPECT_EQ(professor.getSupervisedIds().size(), static_cast<std::size_t>(8));
}

TEST(ProfessorTest, PublicationsAndGrants) {
    Professor professor = makeProfessor();
    professor.publishPaper();
    EXPECT_EQ(professor.getPublications(), 1);
    professor.receiveGrant(Money::fromUnits(500));
    professor.spendGrant(Money::fromUnits(300));
    EXPECT_EQ(professor.getGrantFunds(), Money::fromUnits(1200));
    EXPECT_THROW(professor.spendGrant(Money::fromUnits(5000)), InvalidMoneyOperationException);
}

TEST(DeanTest, ExpulsionReviewAndLeave) {
    Dean dean = makeDean();
    Student student = makeStudent();
    EXPECT_TRUE(dean.canApproveLeaveFor(student));
    EXPECT_THROW(dean.reviewExpulsion(student), AcademicProbationException);
    student.registerFailedExam();
    student.registerFailedExam();
    student.registerFailedExam();
    dean.reviewExpulsion(student);
    EXPECT_EQ(student.getStatus(), StudentStatus::Expelled);
    EXPECT_FALSE(dean.canApproveLeaveFor(student));
    EXPECT_EQ(dean.getSignedDocuments(), 1);
    EXPECT_EQ(dean.getFacultyName(), std::string("ФКСиС"));
}

TEST(MethodistTest, VerificationAndDocuments) {
    Methodist methodist(200, makePerson("Ольга Лис", 1985), "Деканат");
    Student student = makeStudent();
    EXPECT_FALSE(methodist.verifyStudentData(student));
    student.assignGroup("CS-101");
    EXPECT_TRUE(methodist.verifyStudentData(student));
    EXPECT_TRUE(methodist.canIssueCertificateTo(student));
    student.expel();
    EXPECT_FALSE(methodist.canIssueCertificateTo(student));
    methodist.registerDocument();
    EXPECT_EQ(methodist.getProcessedDocuments(), 1);
    EXPECT_EQ(methodist.getEmployeeId(), 200);
    EXPECT_EQ(methodist.getOffice(), std::string("Деканат"));
}
