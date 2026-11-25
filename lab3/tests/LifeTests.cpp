#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

TEST(CertificateTest, ValidityAndVerification) {
    const Student student = makeStudent();
    Certificate certificate(5, student, "С места учёбы", today(), 30);
    EXPECT_TRUE(certificate.isValidOn(today()));
    EXPECT_FALSE(certificate.isValidOn(Date(2026, 11, 9)));
    EXPECT_FALSE(certificate.isValidOn(Date(2026, 12, 31)));
    EXPECT_EQ(certificate.daysLeft(today()), 30);
    EXPECT_EQ(certificate.daysLeft(Date(2027, 6, 1)), 0);
    EXPECT_NO_THROW(certificate.verify(student, today()));
    EXPECT_THROW(certificate.verify(makeStudent(2), today()), DocumentException);
    EXPECT_THROW(certificate.verify(student, Date(2027, 1, 1)), DocumentException);
    certificate.extend(60);
    EXPECT_TRUE(certificate.isValidOn(Date(2027, 1, 1)));
    EXPECT_EQ(certificate.getNumber(), 5);
    EXPECT_EQ(certificate.getStudentId(), 1);
    EXPECT_EQ(certificate.getType(), std::string("С места учёбы"));
    EXPECT_TRUE(certificate.getIssuedOn() == today());
    EXPECT_TRUE(certificate.getValidUntil() == Date(2027, 2, 8));
}

TEST(StudentCardTest, ValidityAndWallet) {
    const Student student = makeStudent();
    StudentCard card(student, "SC-1", Date(2023, 9, 1), Date(2027, 6, 30));
    EXPECT_TRUE(card.isValidOn(today()));
    EXPECT_FALSE(card.isValidOn(Date(2022, 1, 1)));
    card.topUp(Money::fromUnits(20));
    card.pay(Money::fromUnits(5), today());
    EXPECT_EQ(card.getBalance(), Money::fromUnits(15));
    EXPECT_THROW(card.topUp(Money(0)), InvalidMoneyOperationException);
    EXPECT_THROW(card.topUp(Money(-5)), InvalidMoneyOperationException);
    EXPECT_THROW(card.pay(Money::fromUnits(100), today()), InvalidMoneyOperationException);
    EXPECT_THROW(card.pay(Money(1), Date(2028, 1, 1)), DocumentException);
    card.extend(365);
    EXPECT_TRUE(card.isValidOn(Date(2028, 1, 1)));
    EXPECT_EQ(card.getStudentId(), 1);
    EXPECT_EQ(card.getNumber(), std::string("SC-1"));
    EXPECT_TRUE(card.getIssuedOn() == Date(2023, 9, 1));
    EXPECT_TRUE(card.getValidUntil() == Date(2028, 6, 29));
    EXPECT_THROW(StudentCard(student, "X", Date(2026, 1, 1), Date(2025, 1, 1)), DocumentException);
}

TEST(InternshipTest, PeriodAndReport) {
    const Student student = makeStudent();
    Internship internship(1, student, "EPAM", Date(2026, 7, 1), Date(2026, 8, 1), "Иванов");
    EXPECT_EQ(internship.lengthInDays(), 31);
    EXPECT_TRUE(internship.isActiveOn(Date(2026, 7, 15)));
    EXPECT_FALSE(internship.isActiveOn(today()));
    EXPECT_TRUE(internship.isCompletedOn(today()));
    EXPECT_FALSE(internship.hasReport());
    EXPECT_THROW(internship.submitReport(Grade(90), Date(2026, 7, 15)), InvalidExamStateException);
    internship.submitReport(Grade(90), today());
    EXPECT_TRUE(internship.hasReport());
    EXPECT_EQ(internship.getReportScore(), 90);
    EXPECT_EQ(internship.getId(), 1);
    EXPECT_EQ(internship.getStudentId(), 1);
    EXPECT_EQ(internship.getCompany(), std::string("EPAM"));
    EXPECT_TRUE(internship.getStartDate() == Date(2026, 7, 1));
    EXPECT_TRUE(internship.getEndDate() == Date(2026, 8, 1));
    EXPECT_EQ(internship.getSupervisorName(), std::string("Иванов"));
    EXPECT_THROW(Internship(2, student, "x", Date(2026, 8, 1), Date(2026, 7, 1), "y"), InvalidDateException);
}
