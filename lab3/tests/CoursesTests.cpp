#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
CourseOffering offeringWithTeacher(int id, int teacherId, int capacity = 5) {
    return CourseOffering(id, makeCourse(), makeSemester(), makeTeacher(teacherId), capacity);
}
}  // namespace

TEST(CourseTest, DerivedValues) {
    const Course course = makeCourse();
    EXPECT_NEAR(course.hoursPerCredit(), 36.0, 1e-9);
    EXPECT_FALSE(course.isIntensive());
    EXPECT_TRUE(Course("X", "T", 2, 100, "d").isIntensive());
    const Department department("Кафедра ПО", "ФКСиС", makeTeacher());
    EXPECT_TRUE(course.belongsTo(department));
    EXPECT_EQ(course.getCode(), std::string("MATH"));
    EXPECT_EQ(course.getTitle(), std::string("Математика"));
    EXPECT_EQ(course.getCredits(), 6);
    EXPECT_EQ(course.getHours(), 216);
    EXPECT_EQ(course.getDepartmentName(), std::string("Кафедра ПО"));
}

TEST(CourseTest, InvalidDataIsRejected) {
    EXPECT_THROW(Course("", "T", 1, 10, "d"), InvalidCourseDataException);
    EXPECT_THROW(Course("C", "", 1, 10, "d"), InvalidCourseDataException);
    EXPECT_THROW(Course("C", "T", 0, 10, "d"), InvalidCourseDataException);
    EXPECT_THROW(Course("C", "T", 5, 3, "d"), InvalidCourseDataException);
}

TEST(CoursePrerequisiteTest, ChecksRecordBook) {
    const Student student = makeStudent();
    RecordBook book = makeRecordBook(student);
    book.addEntry(RecordEntry("ALG", 4, Grade(70), today(), 1));
    const CoursePrerequisite easy("MATH", "ALG", 60);
    const CoursePrerequisite hard("MATH", "ALG", 80);
    EXPECT_TRUE(easy.isSatisfiedBy(book));
    EXPECT_FALSE(hard.isSatisfiedBy(book));
    EXPECT_EQ(easy.getCourseCode(), std::string("MATH"));
    EXPECT_EQ(easy.getRequiredCourseCode(), std::string("ALG"));
    EXPECT_EQ(easy.getMinPoints(), 60);
}

TEST(CourseOfferingTest, GettersAndCapacityValidation) {
    const CourseOffering offering = makeOffering();
    EXPECT_EQ(offering.getId(), 10);
    EXPECT_EQ(offering.getCourseCode(), std::string("MATH"));
    EXPECT_EQ(offering.getCredits(), 6);
    EXPECT_EQ(offering.getSemesterYear(), 2026);
    EXPECT_TRUE(offering.getTerm() == Term::Autumn);
    EXPECT_EQ(offering.getTeacherId(), 100);
    EXPECT_EQ(offering.getCapacity(), 2);
    EXPECT_THROW(makeOffering(1, 0), InvalidCourseDataException);
}

TEST(CourseOfferingTest, EnrollmentAndLimits) {
    CourseOffering offering = makeOffering();
    const Student first = makeStudent(1);
    const Student second = makeStudent(2);
    const Student third = makeStudent(3);
    offering.enroll(first, makeRecordBook(first));
    EXPECT_TRUE(offering.isEnrolled(first));
    EXPECT_THROW(offering.enroll(first, makeRecordBook(first)), DuplicateRecordException);
    offering.enroll(second, makeRecordBook(second));
    EXPECT_TRUE(offering.isFull());
    EXPECT_EQ(offering.seatsLeft(), 0);
    EXPECT_THROW(offering.enroll(third, makeRecordBook(third)), CourseFullException);
    offering.drop(second);
    EXPECT_EQ(offering.seatsLeft(), 1);
    EXPECT_THROW(offering.drop(second), NotFoundException);
    EXPECT_EQ(offering.getEnrolledIds().size(), static_cast<std::size_t>(1));
}

TEST(CourseOfferingTest, ProbationAndStatusBlockEnrollment) {
    CourseOffering offering = makeOffering();
    Student debtor = makeStudent(1);
    debtor.registerFailedExam();
    debtor.registerFailedExam();
    debtor.registerFailedExam();
    EXPECT_THROW(offering.enroll(debtor, makeRecordBook(debtor)), AcademicProbationException);
    Student expelled = makeStudent(2);
    expelled.expel();
    EXPECT_THROW(offering.enroll(expelled, makeRecordBook(expelled)), InvalidStudentDataException);
}

TEST(CourseOfferingTest, PrerequisitesAreEnforced) {
    CourseOffering offering = makeOffering();
    offering.addPrerequisite(CoursePrerequisite("MATH", "ALG", 60));
    const Student student = makeStudent();
    RecordBook book = makeRecordBook(student);
    EXPECT_THROW(offering.enroll(student, book), PrerequisiteNotMetException);
    book.addEntry(RecordEntry("ALG", 4, Grade(70), today(), 1));
    EXPECT_NO_THROW(offering.enroll(student, book));
}

TEST(LessonTest, PropertiesAndValidation) {
    Lesson lesson = makeLesson();
    EXPECT_EQ(lesson.getId(), 1);
    EXPECT_EQ(lesson.getOfferingId(), 10);
    EXPECT_EQ(lesson.getTeacherId(), 100);
    EXPECT_EQ(lesson.getClassroomLabel(), std::string("Корпус 1-101"));
    EXPECT_TRUE(lesson.isLecture());
    EXPECT_EQ(lesson.getKind(), std::string("lecture"));
    EXPECT_EQ(lesson.durationHours(), 2);
    EXPECT_TRUE(lesson.isOnDay(Weekday::Monday));
    EXPECT_FALSE(lesson.isOnDay(Weekday::Friday));
    lesson.reschedule(LessonTime(Weekday::Friday, 10, 2));
    EXPECT_TRUE(lesson.isOnDay(Weekday::Friday));
    EXPECT_EQ(lesson.getTime().getStartHour(), 10);
    EXPECT_THROW(Lesson(2, makeOffering(), makeClassroom(), LessonTime(Weekday::Monday, 9, 2), "party"),
                 InvalidCourseDataException);
    EXPECT_FALSE(Lesson(3, makeOffering(), makeClassroom(), LessonTime(Weekday::Monday, 9, 2), "practice").isLecture());
    EXPECT_NO_THROW(Lesson(4, makeOffering(), makeClassroom(), LessonTime(Weekday::Monday, 9, 2), "lab"));
}

TEST(LessonTest, RoomMustFitEnrolledStudents) {
    CourseOffering offering = makeOffering();
    const Student first = makeStudent(1);
    const Student second = makeStudent(2);
    offering.enroll(first, makeRecordBook(first));
    offering.enroll(second, makeRecordBook(second));
    EXPECT_THROW(Lesson(1, offering, makeClassroom("102", 1), LessonTime(Weekday::Monday, 9, 2), "lecture"),
                 RoomCapacityException);
}

TEST(LessonTest, ConflictRules) {
    const Lesson base = makeLesson(1, "101", Weekday::Monday, 9);
    const Lesson sameTeacher = makeLesson(2, "102", Weekday::Monday, 10);
    EXPECT_TRUE(base.conflictsWith(sameTeacher));
    const Lesson otherTeacherOtherRoom(3, offeringWithTeacher(11, 200), makeClassroom("103"),
                                       LessonTime(Weekday::Monday, 9, 2), "lecture");
    EXPECT_FALSE(base.conflictsWith(otherTeacherOtherRoom));
    const Lesson otherTeacherSameRoom(4, offeringWithTeacher(12, 201), makeClassroom("101"),
                                      LessonTime(Weekday::Monday, 9, 2), "lecture");
    EXPECT_TRUE(base.conflictsWith(otherTeacherSameRoom));
    EXPECT_FALSE(base.conflictsWith(makeLesson(5, "101", Weekday::Tuesday, 9)));
}

TEST(TimetableTest, AddRemoveAndQueries) {
    Timetable timetable(makeSemester());
    timetable.addLesson(makeLesson(1, "101", Weekday::Monday, 9));
    timetable.addLesson(makeLesson(2, "101", Weekday::Tuesday, 9));
    EXPECT_THROW(timetable.addLesson(makeLesson(3, "102", Weekday::Monday, 10)), ScheduleConflictException);
    EXPECT_EQ(timetable.getLessons().size(), static_cast<std::size_t>(2));
    EXPECT_EQ(timetable.lessonsOn(Weekday::Monday).size(), static_cast<std::size_t>(1));
    EXPECT_EQ(timetable.lessonsOfTeacher(100).size(), static_cast<std::size_t>(2));
    EXPECT_TRUE(timetable.lessonsOfTeacher(999).empty());
    EXPECT_EQ(timetable.totalHoursFor(10), 4);
    EXPECT_EQ(timetable.totalHoursFor(99), 0);
    timetable.removeLesson(1);
    EXPECT_EQ(timetable.getLessons().size(), static_cast<std::size_t>(1));
    EXPECT_THROW(timetable.removeLesson(99), NotFoundException);
    EXPECT_EQ(timetable.getSemesterYear(), 2026);
    EXPECT_FALSE(timetable.hasConflict(makeLesson(4, "101", Weekday::Wednesday, 9)));
}

TEST(AssignmentTest, DeadlinesAndPenalties) {
    const Assignment assignment(1, makeOffering(), "Лаба 1", Date(2026, 11, 10), 20);
    EXPECT_FALSE(assignment.isOverdueOn(today()));
    EXPECT_TRUE(assignment.isOverdueOn(Date(2026, 11, 12)));
    EXPECT_EQ(assignment.latePenaltyPercent(today()), 0);
    EXPECT_EQ(assignment.latePenaltyPercent(Date(2026, 11, 12)), 20);
    EXPECT_EQ(assignment.latePenaltyPercent(Date(2027, 3, 1)), 100);
    EXPECT_TRUE(assignment.acceptsSubmissionOn(Date(2026, 11, 13)));
    EXPECT_FALSE(assignment.acceptsSubmissionOn(Date(2026, 11, 14)));
    EXPECT_EQ(assignment.getId(), 1);
    EXPECT_EQ(assignment.getOfferingId(), 10);
    EXPECT_EQ(assignment.getTitle(), std::string("Лаба 1"));
    EXPECT_TRUE(assignment.getDueDate() == Date(2026, 11, 10));
    EXPECT_EQ(assignment.getMaxScore(), 20);
    EXPECT_THROW(Assignment(2, makeOffering(), "x", today(), 0), InvalidGradeException);
}

TEST(SubmissionTest, GradingWithPenalty) {
    const Assignment assignment(1, makeOffering(), "Лаба 1", Date(2026, 11, 10), 20);
    const Student student = makeStudent();
    Submission onTime(assignment, student, today());
    EXPECT_FALSE(onTime.isGraded());
    EXPECT_NEAR(onTime.percentOfMax(assignment), 0.0, 1e-9);
    onTime.grade(15, assignment);
    EXPECT_EQ(onTime.getScore(), 15);
    EXPECT_NEAR(onTime.percentOfMax(assignment), 75.0, 1e-9);
    Submission late(assignment, student, Date(2026, 11, 12));
    late.grade(20, assignment);
    EXPECT_EQ(late.getPenaltyPercent(), 20);
    EXPECT_EQ(late.getScore(), 16);
    EXPECT_THROW(late.grade(21, assignment), InvalidGradeException);
    EXPECT_THROW(late.grade(-1, assignment), InvalidGradeException);
    EXPECT_EQ(late.getAssignmentId(), 1);
    EXPECT_EQ(late.getStudentId(), 1);
    EXPECT_TRUE(late.getSubmittedOn() == Date(2026, 11, 12));
}

TEST(SubmissionTest, TooLateSubmissionIsRejected) {
    const Assignment assignment(1, makeOffering(), "Лаба 1", Date(2026, 11, 10), 20);
    EXPECT_THROW(Submission(assignment, makeStudent(), Date(2026, 11, 20)), DocumentException);
}
