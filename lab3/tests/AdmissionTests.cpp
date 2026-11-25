#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
Applicant makeApplicant(int id = 1, std::vector<int> scores = {80, 90, 70}) {
    return Applicant(id, makePerson("Абитуриент", 2008), std::move(scores), 4.5);
}

AdmissionCampaign makeCampaign(int quota = 1) {
    return AdmissionCampaign(2026, makeProgram(), quota, 200, Date(2026, 7, 20));
}
}  // namespace

TEST(ApplicantTest, ScoresAndValidation) {
    const Applicant applicant = makeApplicant();
    EXPECT_EQ(applicant.totalScore(), 240);
    EXPECT_EQ(applicant.bestScore(), 90);
    EXPECT_NEAR(applicant.averageScore(), 80.0, 1e-9);
    EXPECT_TRUE(applicant.meetsMinimum(200));
    EXPECT_FALSE(applicant.meetsMinimum(250));
    EXPECT_EQ(applicant.getId(), 1);
    EXPECT_EQ(applicant.getExamScores().size(), static_cast<std::size_t>(3));
    EXPECT_NEAR(applicant.getSchoolAverage(), 4.5, 1e-9);
    EXPECT_THROW(makeApplicant(2, {}), AdmissionException);
    EXPECT_THROW(makeApplicant(2, {101}), InvalidGradeException);
    EXPECT_THROW(makeApplicant(2, {-5}), InvalidGradeException);
}

TEST(AdmissionCampaignTest, AcceptanceRules) {
    AdmissionCampaign campaign = makeCampaign();
    const Applicant applicant = makeApplicant(1);
    campaign.accept(applicant, Date(2026, 7, 1));
    EXPECT_TRUE(campaign.isAccepted(applicant));
    EXPECT_EQ(campaign.remainingPlaces(), 0);
    EXPECT_NEAR(campaign.fillPercent(), 100.0, 1e-9);
    EXPECT_THROW(campaign.accept(applicant, Date(2026, 7, 1)), DuplicateRecordException);
    EXPECT_THROW(campaign.accept(makeApplicant(2), Date(2026, 7, 1)), AdmissionException);
    EXPECT_EQ(campaign.getAcceptedIds().size(), static_cast<std::size_t>(1));
}

TEST(AdmissionCampaignTest, ClosedAndLowScores) {
    AdmissionCampaign campaign = makeCampaign(5);
    EXPECT_THROW(campaign.accept(makeApplicant(1), Date(2026, 8, 1)), AdmissionException);
    EXPECT_THROW(campaign.accept(makeApplicant(2, {50, 50, 50}), Date(2026, 7, 1)), AdmissionException);
    EXPECT_TRUE(campaign.isOpenOn(Date(2026, 7, 20)));
    EXPECT_FALSE(campaign.isOpenOn(Date(2026, 7, 21)));
    EXPECT_EQ(campaign.getYear(), 2026);
    EXPECT_EQ(campaign.getProgramCode(), std::string("CS"));
    EXPECT_EQ(campaign.getQuota(), 5);
    EXPECT_EQ(campaign.getMinScore(), 200);
    EXPECT_TRUE(campaign.getDeadline() == Date(2026, 7, 20));
    EXPECT_THROW(makeCampaign(0), AdmissionException);
}

TEST(ApplicationTest, StatusAndPriority) {
    const Applicant applicant = makeApplicant();
    Application first(1, applicant, makeProgram(), 1, Date(2026, 7, 1));
    Application second(2, applicant, makeProgram("SE"), 2, Date(2026, 7, 1));
    EXPECT_TRUE(first.isHigherPriorityThan(second));
    EXPECT_FALSE(second.isHigherPriorityThan(first));
    first.approve();
    EXPECT_EQ(first.getStatus(), ApplicationStatus::Accepted);
    EXPECT_THROW(first.approve(), AdmissionException);
    second.reject();
    EXPECT_EQ(second.getStatus(), ApplicationStatus::Rejected);
    EXPECT_THROW(second.reject(), AdmissionException);
    EXPECT_THROW(Application(3, applicant, makeProgram(), 0, today()), AdmissionException);
    EXPECT_EQ(first.getId(), 1);
    EXPECT_EQ(first.getApplicantId(), 1);
    EXPECT_EQ(first.getProgramCode(), std::string("CS"));
    EXPECT_EQ(first.getPriority(), 1);
    EXPECT_TRUE(first.getSubmittedOn() == Date(2026, 7, 1));
}

TEST(ApplicationTest, EvaluationAgainstCampaign) {
    AdmissionCampaign campaign = makeCampaign(5);
    const Applicant strong = makeApplicant(1);
    const Applicant weak = makeApplicant(2, {50, 50, 50});
    Application good(1, strong, makeProgram(), 1, Date(2026, 7, 1));
    Application bad(2, weak, makeProgram(), 1, Date(2026, 7, 1));
    EXPECT_TRUE(good.evaluate(campaign, strong, Date(2026, 7, 1)));
    EXPECT_EQ(good.getStatus(), ApplicationStatus::Accepted);
    EXPECT_FALSE(bad.evaluate(campaign, weak, Date(2026, 7, 1)));
    EXPECT_EQ(bad.getStatus(), ApplicationStatus::Rejected);
}

TEST(EnrollmentTest, LifecycleAndDays) {
    const Student student = makeStudent();
    Enrollment enrollment(student, makeOffering(), Date(2026, 9, 1));
    EXPECT_TRUE(enrollment.isActive());
    EXPECT_EQ(enrollment.daysEnrolled(today()), 70);
    enrollment.complete();
    EXPECT_EQ(enrollment.getStatus(), EnrollmentStatus::Completed);
    EXPECT_THROW(enrollment.drop(), InvalidExamStateException);
    Enrollment other(student, makeOffering(), today());
    other.drop();
    EXPECT_FALSE(other.isActive());
    EXPECT_THROW(other.complete(), InvalidExamStateException);
    EXPECT_EQ(enrollment.getStudentId(), 1);
    EXPECT_EQ(enrollment.getOfferingId(), 10);
    EXPECT_TRUE(enrollment.getEnrolledOn() == Date(2026, 9, 1));
}

TEST(AcademicLeaveTest, ValidationAndExtension) {
    const Student student = makeStudent();
    AcademicLeave leave(student, Date(2026, 11, 1), Date(2027, 3, 1), "Болезнь");
    EXPECT_EQ(leave.lengthInDays(), 120);
    EXPECT_TRUE(leave.isActiveOn(Date(2027, 1, 1)));
    EXPECT_FALSE(leave.isActiveOn(Date(2027, 4, 1)));
    leave.extend(100);
    EXPECT_EQ(leave.lengthInDays(), 220);
    EXPECT_THROW(leave.extend(300), InvalidDateException);
    EXPECT_THROW(AcademicLeave(student, Date(2026, 11, 1), Date(2026, 10, 1), "x"), InvalidDateException);
    EXPECT_THROW(AcademicLeave(student, Date(2026, 1, 1), Date(2028, 1, 1), "x"), InvalidDateException);
    Student expelled = makeStudent(2);
    expelled.expel();
    EXPECT_THROW(AcademicLeave(expelled, Date(2026, 11, 1), Date(2027, 1, 1), "x"), InvalidStudentDataException);
    EXPECT_EQ(leave.getStudentId(), 1);
    EXPECT_EQ(leave.getReason(), std::string("Болезнь"));
    EXPECT_TRUE(leave.getStartDate() == Date(2026, 11, 1));
    EXPECT_TRUE(leave.getEndDate() == Date(2027, 6, 9));
}

TEST(AcademicLeaveTest, DeanApproval) {
    AcademicLeave leave(makeStudent(), Date(2026, 11, 1), Date(2027, 1, 1), "Семья");
    EXPECT_FALSE(leave.isApproved());
    leave.approve(makeDean());
    EXPECT_TRUE(leave.isApproved());
    EXPECT_EQ(leave.getApprovedById(), 102);
}

TEST(TransferRequestTest, ApprovalAndApplication) {
    Student student = makeStudent();
    TransferRequest request(1, student, makeProgram("SE"), "Интерес");
    EXPECT_TRUE(request.isPending());
    EXPECT_THROW(request.applyTo(student), AdmissionException);
    request.approve(makeDean());
    EXPECT_THROW(request.approve(makeDean()), AdmissionException);
    request.applyTo(student);
    EXPECT_EQ(student.getProgramCode(), std::string("SE"));
    EXPECT_EQ(request.getStatus(), RequestStatus::Applied);
    EXPECT_EQ(request.getApprovedById(), 102);
    EXPECT_EQ(request.getId(), 1);
    EXPECT_EQ(request.getStudentId(), 1);
    EXPECT_EQ(request.getFromProgramCode(), std::string("CS"));
    EXPECT_EQ(request.getToProgramCode(), std::string("SE"));
    EXPECT_EQ(request.getReason(), std::string("Интерес"));
}

TEST(TransferRequestTest, RejectionAndSameProgram) {
    const Student student = makeStudent();
    TransferRequest request(2, student, makeProgram("SE"), "x");
    request.reject();
    EXPECT_EQ(request.getStatus(), RequestStatus::Rejected);
    EXPECT_THROW(request.reject(), AdmissionException);
    EXPECT_THROW(TransferRequest(3, student, makeProgram("CS"), "x"), InvalidStudentDataException);
}

TEST(AdministrativeOrderTest, SigningAndAge) {
    AdministrativeOrder order(15, "Отчисление", Date(2026, 11, 1), "Текст");
    EXPECT_FALSE(order.isSigned());
    order.sign(makeDean());
    EXPECT_TRUE(order.isSigned());
    EXPECT_EQ(order.getSignedById(), 102);
    EXPECT_THROW(order.sign(makeDean()), DocumentException);
    EXPECT_TRUE(order.isRecentOn(today(), 30));
    EXPECT_FALSE(order.isRecentOn(today(), 5));
    EXPECT_NE(order.describe().find("№15"), std::string::npos);
    EXPECT_EQ(order.getNumber(), 15);
    EXPECT_EQ(order.getType(), std::string("Отчисление"));
    EXPECT_TRUE(order.getIssuedOn() == Date(2026, 11, 1));
    EXPECT_EQ(order.getText(), std::string("Текст"));
}
