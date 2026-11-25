#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
Exam makeExam(int capacity = 2) { return Exam(1, makeOffering(), Date(2026, 12, 22), makeClassroom("101", capacity)); }

ExamResult makeResult(int points, const Student& student) {
    Exam exam = makeExam();
    exam.registerStudent(student);
    exam.start();
    return ExamResult(exam, student, Grade(points));
}

Thesis makeDefendedThesis(const Student& student, Professor& professor) {
    professor.supervise(student);
    Thesis thesis(1, student, professor, "Нейросети", Date(2027, 6, 1));
    thesis.submit(today());
    thesis.defend(student, Grade(90));
    return thesis;
}
}  // namespace

TEST(ExamTest, RegistrationAndStates) {
    Exam exam = makeExam();
    const Student first = makeStudent(1);
    const Student second = makeStudent(2);
    exam.registerStudent(first);
    EXPECT_THROW(exam.registerStudent(first), DuplicateRecordException);
    exam.registerStudent(second);
    EXPECT_THROW(exam.registerStudent(makeStudent(3)), RoomCapacityException);
    EXPECT_EQ(exam.attendeeCount(), 2);
    EXPECT_TRUE(exam.isRegistered(first));
    EXPECT_THROW(exam.finish(), InvalidExamStateException);
    exam.start();
    EXPECT_THROW(exam.start(), InvalidExamStateException);
    EXPECT_THROW(exam.registerStudent(makeStudent(4)), InvalidExamStateException);
    exam.finish();
    EXPECT_EQ(exam.getStatus(), ExamStatus::Finished);
    EXPECT_EQ(exam.getId(), 1);
    EXPECT_EQ(exam.getOfferingId(), 10);
    EXPECT_TRUE(exam.getDate() == Date(2026, 12, 22));
    EXPECT_EQ(exam.getClassroomLabel(), std::string("Корпус 1-101"));
}

TEST(ExamResultTest, RequiresStartedExamAndRegistration) {
    Exam exam = makeExam();
    const Student student = makeStudent();
    exam.registerStudent(student);
    EXPECT_THROW(ExamResult(exam, student, Grade(80)), InvalidExamStateException);
    exam.start();
    EXPECT_THROW(ExamResult(exam, makeStudent(2), Grade(80)), NotFoundException);
    const ExamResult result(exam, student, Grade(80));
    EXPECT_TRUE(result.isPassed());
    EXPECT_EQ(result.getExamId(), 1);
    EXPECT_EQ(result.getStudentId(), 1);
    EXPECT_EQ(result.getGrade().getPoints(), 80);
    EXPECT_TRUE(result.getTakenOn() == Date(2026, 12, 22));
}

TEST(ExamResultTest, RecordEntryAndStanding) {
    Student student = makeStudent();
    const ExamResult failed = makeResult(40, student);
    failed.updateStanding(student);
    EXPECT_EQ(student.getFailedExams(), 1);
    const ExamResult passed = makeResult(80, student);
    passed.updateStanding(student);
    EXPECT_EQ(student.getFailedExams(), 0);
    const RecordEntry entry = passed.toRecordEntry(makeCourse(), 2);
    EXPECT_EQ(entry.getCourseCode(), std::string("MATH"));
    EXPECT_EQ(entry.getAttempt(), 2);
    EXPECT_FALSE(failed.isPassed());
}

TEST(AttendanceRecordTest, WeightsAndFlags) {
    const Lesson lesson = makeLesson();
    const Student student = makeStudent();
    const AttendanceRecord present(lesson, student, AttendanceStatus::Present, today());
    const AttendanceRecord late(lesson, student, AttendanceStatus::Late, today());
    const AttendanceRecord excused(lesson, student, AttendanceStatus::Excused, today());
    const AttendanceRecord absent(lesson, student, AttendanceStatus::Absent, today());
    EXPECT_NEAR(present.weight(), 1.0, 1e-9);
    EXPECT_NEAR(late.weight(), 0.5, 1e-9);
    EXPECT_NEAR(excused.weight(), 1.0, 1e-9);
    EXPECT_NEAR(absent.weight(), 0.0, 1e-9);
    EXPECT_TRUE(present.countsAsPresent());
    EXPECT_TRUE(late.countsAsPresent());
    EXPECT_FALSE(absent.countsAsPresent());
    EXPECT_TRUE(excused.isExcused());
    EXPECT_FALSE(present.isExcused());
    EXPECT_EQ(present.getLessonId(), 1);
    EXPECT_EQ(present.getStudentId(), 1);
    EXPECT_TRUE(present.getStatus() == AttendanceStatus::Present);
    EXPECT_TRUE(present.getDate() == today());
}

TEST(AttendanceJournalTest, PercentagesAndAdmission) {
    AttendanceJournal journal(makeOffering());
    const Student student = makeStudent(1);
    const Student other = makeStudent(2);
    EXPECT_NEAR(journal.attendancePercent(student), 100.0, 1e-9);
    EXPECT_TRUE(journal.isAllowedToExam(student));
    journal.mark(AttendanceRecord(makeLesson(1), student, AttendanceStatus::Present, today()));
    journal.mark(AttendanceRecord(makeLesson(2), student, AttendanceStatus::Absent, today()));
    EXPECT_NEAR(journal.attendancePercent(student), 50.0, 1e-9);
    EXPECT_FALSE(journal.isAllowedToExam(student));
    EXPECT_EQ(journal.absencesOf(student), 1);
    EXPECT_EQ(journal.absencesOf(other), 0);
    EXPECT_THROW(journal.mark(AttendanceRecord(makeLesson(1), student, AttendanceStatus::Late, today())),
                 DuplicateRecordException);
    const std::vector<Student> all = {student, other};
    EXPECT_EQ(journal.studentsBelow(60.0, all).size(), static_cast<std::size_t>(1));
    EXPECT_EQ(journal.getRecords().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(journal.getOfferingId(), 10);
}

TEST(RetakeTest, AttemptsAndScheduling) {
    const Student student = makeStudent();
    const ExamResult failed = makeResult(30, student);
    Retake retake(failed, Date(2027, 1, 10), 2);
    EXPECT_FALSE(retake.isLastAttempt());
    EXPECT_EQ(retake.nextAttempt(), 3);
    EXPECT_TRUE(Retake(failed, Date(2027, 1, 10), 3).isLastAttempt());
    EXPECT_EQ(retake.daysUntil(Date(2027, 1, 1)), 9);
    retake.reschedule(Date(2027, 1, 15));
    EXPECT_TRUE(retake.getScheduledOn() == Date(2027, 1, 15));
    EXPECT_THROW(retake.reschedule(Date(2027, 1, 1)), InvalidDateException);
    EXPECT_THROW(Retake(failed, today(), 1), InvalidExamStateException);
    EXPECT_THROW(Retake(failed, today(), 4), InvalidExamStateException);
    EXPECT_THROW(Retake(makeResult(80, student), today(), 2), InvalidExamStateException);
    EXPECT_EQ(retake.getStudentId(), 1);
    EXPECT_EQ(retake.getExamId(), 1);
    EXPECT_EQ(retake.getAttempt(), 2);
}

TEST(ThesisTest, RequiresSupervisor) {
    Professor professor = makeProfessor();
    const Student student = makeStudent();
    EXPECT_THROW(Thesis(1, student, professor, "Тема", today()), InvalidStudentDataException);
}

TEST(ThesisTest, SubmissionAndDefense) {
    Professor professor = makeProfessor();
    const Student student = makeStudent();
    professor.supervise(student);
    Thesis thesis(1, student, professor, "Нейросети", Date(2027, 6, 1));
    EXPECT_THROW(thesis.defend(student, Grade(90)), InvalidExamStateException);
    EXPECT_FALSE(thesis.isLate(today()));
    thesis.submit(today());
    EXPECT_THROW(thesis.submit(today()), InvalidExamStateException);
    thesis.defend(student, Grade(90));
    EXPECT_TRUE(thesis.isDefended());
    EXPECT_EQ(thesis.getScore(), 90);
    EXPECT_EQ(thesis.getId(), 1);
    EXPECT_EQ(thesis.getStudentId(), 1);
    EXPECT_EQ(thesis.getSupervisorId(), 101);
    EXPECT_EQ(thesis.getTitle(), std::string("Нейросети"));
    EXPECT_TRUE(thesis.getDeadline() == Date(2027, 6, 1));
}

TEST(ThesisTest, LateFailingAndProbation) {
    Professor professor = makeProfessor();
    Student student = makeStudent();
    professor.supervise(student);
    Thesis late(1, student, professor, "Тема", Date(2026, 1, 1));
    EXPECT_THROW(late.submit(today()), DocumentException);
    Thesis failing(2, student, professor, "Тема", Date(2027, 6, 1));
    failing.submit(today());
    failing.defend(student, Grade(30));
    EXPECT_EQ(failing.getStatus(), ThesisStatus::Rejected);
    Thesis blocked(3, student, professor, "Тема", Date(2027, 6, 1));
    blocked.submit(today());
    student.registerFailedExam();
    student.registerFailedExam();
    student.registerFailedExam();
    EXPECT_THROW(blocked.defend(student, Grade(80)), AcademicProbationException);
    blocked.reject();
    EXPECT_EQ(blocked.getStatus(), ThesisStatus::Rejected);
}

TEST(DiplomaTest, IssuedForGraduate) {
    Professor professor = makeProfessor();
    const Student student = makeStudent();
    const Thesis thesis = makeDefendedThesis(student, professor);
    const StudyProgram program("P", "Проверка", 1, 10, "bachelor");
    RecordBook book = makeRecordBook(student);
    book.addEntry(RecordEntry("A", 10, Grade(95), today(), 1));
    const Diploma diploma(student, program, book, thesis, Date(2027, 7, 1));
    EXPECT_TRUE(diploma.isWithHonors());
    EXPECT_EQ(diploma.honorsLabel(), std::string("с отличием"));
    EXPECT_EQ(diploma.getNumber(), std::string("P-2027-1"));
    EXPECT_TRUE(diploma.isIssuedIn(2027));
    EXPECT_FALSE(diploma.isIssuedIn(2026));
    EXPECT_NE(diploma.describe().find("Нейросети"), std::string::npos);
    EXPECT_NEAR(diploma.getGpa(), 95.0, 1e-9);
    EXPECT_EQ(diploma.getStudentId(), 1);
    EXPECT_EQ(diploma.getProgramCode(), std::string("P"));
    EXPECT_TRUE(diploma.getIssuedOn() == Date(2027, 7, 1));
    EXPECT_EQ(diploma.getThesisTitle(), std::string("Нейросети"));
}

TEST(DiplomaTest, RequiresThesisAndCredits) {
    Professor professor = makeProfessor();
    const Student student = makeStudent();
    const Thesis defended = makeDefendedThesis(student, professor);
    const StudyProgram program("P", "Проверка", 1, 10, "bachelor");
    RecordBook small = makeRecordBook(student);
    small.addEntry(RecordEntry("A", 4, Grade(70), today(), 1));
    EXPECT_THROW(Diploma(student, program, small, defended, today()), InsufficientCreditsException);
    const Thesis draft(2, student, professor, "Тема", today());
    RecordBook full = makeRecordBook(student);
    full.addEntry(RecordEntry("A", 10, Grade(70), today(), 1));
    EXPECT_THROW(Diploma(student, program, full, draft, today()), InvalidExamStateException);
    EXPECT_EQ(Diploma(student, program, full, defended, today()).honorsLabel(), std::string("без отличия"));
}
