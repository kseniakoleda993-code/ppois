#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(GradeTest, RangeAndScales) {
    EXPECT_THROW(Grade(-1), InvalidGradeException);
    EXPECT_THROW(Grade(101), InvalidGradeException);
    EXPECT_EQ(Grade(95).toFivePoint(), 5);
    EXPECT_EQ(Grade(80).toFivePoint(), 4);
    EXPECT_EQ(Grade(65).toFivePoint(), 3);
    EXPECT_EQ(Grade(40).toFivePoint(), 2);
    EXPECT_TRUE(Grade(60).isPassing());
    EXPECT_FALSE(Grade(59).isPassing());
    EXPECT_EQ(Grade(0).getPoints(), 0);
}

TEST(GradeTest, Letters) {
    EXPECT_EQ(Grade(95).letter(), std::string("A"));
    EXPECT_EQ(Grade(85).letter(), std::string("B"));
    EXPECT_EQ(Grade(72).letter(), std::string("C"));
    EXPECT_EQ(Grade(61).letter(), std::string("D"));
    EXPECT_EQ(Grade(10).letter(), std::string("F"));
}

TEST(RecordEntryTest, PassedAndWeight) {
    const RecordEntry entry("MATH", 6, Grade(80), today(), 1);
    EXPECT_TRUE(entry.isPassed());
    EXPECT_EQ(entry.weightedPoints(), 480LL);
    EXPECT_EQ(entry.getCourseCode(), std::string("MATH"));
    EXPECT_EQ(entry.getCredits(), 6);
    EXPECT_EQ(entry.getGrade().getPoints(), 80);
    EXPECT_TRUE(entry.getTakenOn() == today());
    EXPECT_EQ(entry.getAttempt(), 1);
    EXPECT_FALSE(RecordEntry("X", 1, Grade(10), today(), 1).isPassed());
}

TEST(RecordBookTest, CreditsAndGpa) {
    const Student student = makeStudent();
    RecordBook book = makeRecordBook(student);
    EXPECT_EQ(book.getStudentId(), 1);
    EXPECT_NEAR(book.gpa(), 0.0, 1e-9);
    EXPECT_FALSE(book.hasHonors());
    book.addEntry(RecordEntry("MATH", 6, Grade(90), today(), 1));
    book.addEntry(RecordEntry("PHYS", 4, Grade(40), today(), 1));
    EXPECT_EQ(book.earnedCredits(), 6);
    EXPECT_NEAR(book.gpa(), 70.0, 1e-9);
    EXPECT_EQ(book.failedCourses().size(), static_cast<std::size_t>(1));
    EXPECT_FALSE(book.hasHonors());
    EXPECT_EQ(book.getEntries().size(), static_cast<std::size_t>(2));
}

TEST(RecordBookTest, RetakeKeepsBestScore) {
    const Student student = makeStudent();
    RecordBook book = makeRecordBook(student);
    book.addEntry(RecordEntry("MATH", 6, Grade(50), today(), 1));
    book.addEntry(RecordEntry("MATH", 6, Grade(75), today(), 2));
    book.addEntry(RecordEntry("MATH", 6, Grade(30), today(), 3));
    EXPECT_EQ(book.bestScore("MATH"), 75);
    EXPECT_EQ(book.getEntries().size(), static_cast<std::size_t>(1));
    EXPECT_THROW(book.bestScore("NOPE"), NotFoundException);
}

TEST(RecordBookTest, PassedAndHonors) {
    const Student student = makeStudent();
    RecordBook book = makeRecordBook(student);
    book.addEntry(RecordEntry("MATH", 6, Grade(95), today(), 1));
    EXPECT_TRUE(book.hasPassed("MATH", 90));
    EXPECT_FALSE(book.hasPassed("MATH", 99));
    EXPECT_FALSE(book.hasPassed("PHYS", 60));
    EXPECT_TRUE(book.hasHonors());
}
