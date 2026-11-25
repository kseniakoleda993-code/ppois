#include <gtest/gtest.h>

#include <string>

#include "studis/Exceptions.h"

using namespace studis;

namespace {
template <typename Exception>
bool isCaughtAsBaseWithDetails(const std::string& details) {
    try {
        throw Exception(details);
    } catch (const StudentSystemException& error) {
        return std::string(error.what()).find(details) != std::string::npos;
    }
    return false;
}
}  // namespace

TEST(ExceptionsTest, BaseExceptionKeepsMessage) {
    const StudentSystemException error("сообщение");
    EXPECT_EQ(std::string(error.what()), "сообщение");
}

TEST(ExceptionsTest, EveryExceptionDerivesFromBaseAndKeepsDetails) {
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidStudentDataException>("d0"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidGradeException>("d1"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidCourseDataException>("d2"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<CourseFullException>("d3"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<PrerequisiteNotMetException>("d4"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<ScheduleConflictException>("d5"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidDateException>("d6"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InsufficientCreditsException>("d7"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<DuplicateRecordException>("d8"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<TuitionPaymentException>("d9"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidMoneyOperationException>("d10"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<ScholarshipEligibilityException>("d11"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<InvalidExamStateException>("d12"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<AcademicProbationException>("d13"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<RoomCapacityException>("d14"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<DocumentException>("d15"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<LibraryException>("d16"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<AdmissionException>("d17"));
    EXPECT_TRUE(isCaughtAsBaseWithDetails<NotFoundException>("d18"));
}
