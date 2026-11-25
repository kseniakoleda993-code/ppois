#include <gtest/gtest.h>

#include "TestData.h"

using namespace testdata;

namespace {
TuitionContract makeContract(const Student& student) {
    return TuitionContract(1, student, makeProgram(), Money::fromUnits(1000), 4);
}

RecordBook bookWithGrade(const Student& student, int points) {
    RecordBook book = makeRecordBook(student);
    book.addEntry(RecordEntry("MATH", 6, Grade(points), today(), 1));
    return book;
}
}  // namespace

TEST(TuitionContractTest, PaymentsAndInstallments) {
    const Student student = makeStudent();
    TuitionContract contract = makeContract(student);
    EXPECT_EQ(contract.installmentAmount(), Money::fromUnits(250));
    contract.registerPayment(Money::fromUnits(300));
    EXPECT_EQ(contract.paidInstallments(), 1);
    EXPECT_EQ(contract.outstanding(), Money::fromUnits(700));
    EXPECT_FALSE(contract.isFullyPaid());
    EXPECT_THROW(contract.registerPayment(Money::fromUnits(701)), TuitionPaymentException);
    EXPECT_THROW(contract.registerPayment(Money(0)), TuitionPaymentException);
    EXPECT_THROW(contract.registerPayment(Money(-1)), TuitionPaymentException);
    contract.registerPayment(Money::fromUnits(700));
    EXPECT_TRUE(contract.isFullyPaid());
    EXPECT_EQ(contract.getId(), 1);
    EXPECT_EQ(contract.getStudentId(), 1);
    EXPECT_EQ(contract.getProgramCode(), std::string("CS"));
    EXPECT_EQ(contract.getTotalFee(), Money::fromUnits(1000));
    EXPECT_EQ(contract.getPaid(), Money::fromUnits(1000));
    EXPECT_EQ(contract.getInstallments(), 4);
}

TEST(TuitionContractTest, InvalidContractsAreRejected) {
    const Student student = makeStudent();
    EXPECT_THROW(TuitionContract(1, student, makeProgram(), Money(0), 4), TuitionPaymentException);
    EXPECT_THROW(TuitionContract(1, student, makeProgram(), Money(-5), 4), TuitionPaymentException);
    EXPECT_THROW(TuitionContract(1, student, makeProgram(), Money::fromUnits(10), 0), TuitionPaymentException);
}

TEST(TuitionPaymentTest, ValidationAndHelpers) {
    const Student student = makeStudent();
    const TuitionContract contract = makeContract(student);
    const TuitionPayment payment(7, contract, Money::fromUnits(100), today(), "online");
    EXPECT_TRUE(payment.isOnline());
    EXPECT_FALSE(TuitionPayment(8, contract, Money(1), today(), "cash").isOnline());
    EXPECT_TRUE(payment.isLate(Date(2026, 11, 1)));
    EXPECT_FALSE(payment.isLate(Date(2026, 12, 1)));
    EXPECT_NE(payment.receiptText().find("Платёж №7"), std::string::npos);
    EXPECT_EQ(payment.getId(), 7);
    EXPECT_EQ(payment.getContractId(), 1);
    EXPECT_EQ(payment.getAmount(), Money::fromUnits(100));
    EXPECT_TRUE(payment.getPaidOn() == today());
    EXPECT_EQ(payment.getMethod(), std::string("online"));
    EXPECT_THROW(TuitionPayment(9, contract, Money(0), today(), "cash"), TuitionPaymentException);
    EXPECT_THROW(TuitionPayment(9, contract, Money(-1), today(), "cash"), TuitionPaymentException);
    EXPECT_THROW(TuitionPayment(9, contract, Money(1), today(), "bitcoin"), TuitionPaymentException);
    EXPECT_NO_THROW(TuitionPayment(9, contract, Money(1), today(), "bank"));
}

TEST(ScholarshipTest, EligibilityAndAmounts) {
    const Student student = makeStudent();
    const Scholarship scholarship("Повышенная", Money::fromUnits(100), 80.0, 6);
    EXPECT_TRUE(scholarship.isEligible(bookWithGrade(student, 85)));
    EXPECT_FALSE(scholarship.isEligible(bookWithGrade(student, 70)));
    EXPECT_FALSE(scholarship.isEligible(makeRecordBook(student)));
    RecordBook withFail = bookWithGrade(student, 95);
    withFail.addEntry(RecordEntry("PHYS", 1, Grade(10), today(), 1));
    EXPECT_FALSE(scholarship.isEligible(withFail));
    EXPECT_EQ(scholarship.totalValue(), Money::fromUnits(600));
    EXPECT_EQ(scholarship.amountFor(bookWithGrade(student, 85)), Money::fromUnits(100));
    EXPECT_EQ(scholarship.amountFor(bookWithGrade(student, 95)), Money::fromUnits(120));
    EXPECT_EQ(scholarship.getName(), std::string("Повышенная"));
    EXPECT_EQ(scholarship.getMonthlyAmount(), Money::fromUnits(100));
    EXPECT_NEAR(scholarship.getMinGpa(), 80.0, 1e-9);
    EXPECT_EQ(scholarship.getDurationMonths(), 6);
}

TEST(ScholarshipAwardTest, ActivePeriodAndPayments) {
    const Student student = makeStudent();
    const Scholarship scholarship("Повышенная", Money::fromUnits(100), 80.0, 6);
    ScholarshipAward award(student, scholarship, bookWithGrade(student, 85), Date(2026, 9, 1));
    EXPECT_TRUE(award.isActiveOn(today()));
    EXPECT_FALSE(award.isActiveOn(Date(2026, 8, 1)));
    EXPECT_FALSE(award.isActiveOn(Date(2027, 6, 1)));
    EXPECT_EQ(award.totalPaidUntil(today()), Money::fromUnits(200));
    EXPECT_EQ(award.totalPaidUntil(Date(2030, 1, 1)), Money::fromUnits(600));
    EXPECT_EQ(award.totalPaidUntil(Date(2026, 8, 1)), Money(0));
    EXPECT_EQ(award.monthsLeft(today()), 4);
    EXPECT_EQ(award.monthsLeft(Date(2030, 1, 1)), 0);
    award.revoke();
    EXPECT_TRUE(award.isRevoked());
    EXPECT_FALSE(award.isActiveOn(today()));
    EXPECT_EQ(award.monthsLeft(today()), 0);
    EXPECT_EQ(award.getStudentId(), 1);
    EXPECT_EQ(award.getScholarshipName(), std::string("Повышенная"));
    EXPECT_EQ(award.getMonthlyAmount(), Money::fromUnits(100));
    EXPECT_TRUE(award.getStartDate() == Date(2026, 9, 1));
    EXPECT_TRUE(award.getEndDate() == Date(2027, 2, 28));
}

TEST(ScholarshipAwardTest, IneligibleStudentsAreRejected) {
    const Student student = makeStudent();
    const Scholarship scholarship("Повышенная", Money::fromUnits(100), 80.0, 6);
    EXPECT_THROW(ScholarshipAward(student, scholarship, bookWithGrade(student, 50), today()),
                 ScholarshipEligibilityException);
    Student expelled = makeStudent(2);
    expelled.expel();
    EXPECT_THROW(ScholarshipAward(expelled, scholarship, bookWithGrade(expelled, 90), today()),
                 ScholarshipEligibilityException);
}

TEST(DormitoryRoomTest, AssignmentAndVacating) {
    DormitoryRoom room(12, 1);
    const Student first = makeStudent(1);
    room.assign(first);
    EXPECT_TRUE(room.hosts(first));
    EXPECT_TRUE(room.isFull());
    EXPECT_EQ(room.freePlaces(), 0);
    EXPECT_THROW(room.assign(first), DuplicateRecordException);
    EXPECT_THROW(room.assign(makeStudent(2)), RoomCapacityException);
    room.vacate(first);
    EXPECT_EQ(room.freePlaces(), 1);
    EXPECT_EQ(room.getNumber(), 12);
    EXPECT_EQ(room.getCapacity(), 1);
    EXPECT_TRUE(room.getResidentIds().empty());
    EXPECT_THROW(DormitoryRoom(1, 0), RoomCapacityException);
}

TEST(DormitoryTest, SettlingAndEviction) {
    Dormitory dormitory("Общежитие", Address("a", "b", "220000"), Money::fromUnits(25));
    EXPECT_NEAR(dormitory.occupancyPercent(), 0.0, 1e-9);
    dormitory.addRoom(DormitoryRoom(1, 1));
    dormitory.addRoom(DormitoryRoom(2, 2));
    const Student first = makeStudent(1);
    EXPECT_EQ(dormitory.settle(first), 1);
    EXPECT_EQ(dormitory.settle(makeStudent(2)), 2);
    EXPECT_EQ(dormitory.totalFreePlaces(), 1);
    EXPECT_NEAR(dormitory.occupancyPercent(), 200.0 / 3.0, 1e-9);
    dormitory.evict(first);
    EXPECT_EQ(dormitory.totalFreePlaces(), 2);
    EXPECT_EQ(dormitory.feeForMonths(4), Money::fromUnits(100));
    EXPECT_EQ(dormitory.getName(), std::string("Общежитие"));
    EXPECT_TRUE(dormitory.getAddress().isInCity("b"));
    EXPECT_EQ(dormitory.getMonthlyFee(), Money::fromUnits(25));
    EXPECT_EQ(dormitory.getRooms().size(), static_cast<std::size_t>(2));
}

TEST(DormitoryTest, SettlingRestrictions) {
    Dormitory dormitory("Общежитие", Address(), Money::fromUnits(25));
    dormitory.addRoom(DormitoryRoom(1, 1));
    Student debtor = makeStudent(1);
    debtor.registerFailedExam();
    debtor.registerFailedExam();
    debtor.registerFailedExam();
    EXPECT_THROW(dormitory.settle(debtor), AcademicProbationException);
    Student expelled = makeStudent(2);
    expelled.expel();
    EXPECT_THROW(dormitory.settle(expelled), InvalidStudentDataException);
    dormitory.settle(makeStudent(3));
    EXPECT_THROW(dormitory.settle(makeStudent(4)), RoomCapacityException);
}

TEST(BookTest, LendingAndReturning) {
    Book book("978-1", "Книга", "Автор", 1);
    EXPECT_TRUE(book.isAvailable());
    book.lend();
    EXPECT_EQ(book.borrowedCount(), 1);
    EXPECT_THROW(book.lend(), LibraryException);
    book.giveBack();
    EXPECT_THROW(book.giveBack(), LibraryException);
    EXPECT_EQ(book.getIsbn(), std::string("978-1"));
    EXPECT_EQ(book.getTitle(), std::string("Книга"));
    EXPECT_EQ(book.getAuthor(), std::string("Автор"));
    EXPECT_EQ(book.getTotalCopies(), 1);
    EXPECT_EQ(book.getAvailableCopies(), 1);
    EXPECT_THROW(Book("", "t", "a", 1), LibraryException);
    EXPECT_THROW(Book("i", "t", "a", 0), LibraryException);
}

TEST(BookLoanTest, OverdueFinesAndExtensions) {
    const Book book("978-1", "Книга", "Автор", 1);
    BookLoan loan(book, makeStudent(), Date(2026, 11, 1), 14);
    EXPECT_TRUE(loan.getDueOn() == Date(2026, 11, 15));
    EXPECT_FALSE(loan.isOverdueOn(today()));
    EXPECT_TRUE(loan.isOverdueOn(Date(2026, 11, 20)));
    EXPECT_EQ(loan.daysOverdue(Date(2026, 11, 20)), 5);
    EXPECT_EQ(loan.fineOn(Date(2026, 11, 20), Money(50)), Money(250));
    loan.extend(7);
    loan.extend(7);
    EXPECT_EQ(loan.getExtensions(), 2);
    EXPECT_THROW(loan.extend(7), LibraryException);
    loan.markReturned();
    EXPECT_TRUE(loan.isReturned());
    EXPECT_FALSE(loan.isOverdueOn(Date(2027, 1, 1)));
    EXPECT_THROW(loan.extend(1), LibraryException);
    EXPECT_EQ(loan.getIsbn(), std::string("978-1"));
    EXPECT_EQ(loan.getStudentId(), 1);
    EXPECT_TRUE(loan.getBorrowedOn() == Date(2026, 11, 1));
}

TEST(LibraryCardTest, BorrowingRules) {
    const Student student = makeStudent();
    LibraryCard card(student, today());
    Book first("1", "A", "x", 1);
    card.borrow(first, student, today());
    EXPECT_EQ(card.activeLoans(), 1);
    EXPECT_THROW(card.borrow(first, student, today()), LibraryException);
    card.giveBack(first);
    EXPECT_EQ(card.activeLoans(), 0);
    EXPECT_THROW(card.giveBack(first), NotFoundException);
    EXPECT_EQ(card.getStudentId(), 1);
    EXPECT_TRUE(card.getIssuedOn() == today());
    EXPECT_EQ(card.getLoans().size(), static_cast<std::size_t>(1));
}

TEST(LibraryCardTest, LimitAndOverdueBlockBorrowing) {
    const Student student = makeStudent();
    LibraryCard card(student, today());
    std::vector<Book> books;
    for (int index = 0; index < 6; ++index) {
        books.emplace_back(std::to_string(index), "T", "A", 1);
    }
    for (int index = 0; index < 5; ++index) {
        card.borrow(books[index], student, today());
    }
    EXPECT_THROW(card.borrow(books[5], student, today()), LibraryException);
    LibraryCard overdueCard(student, today());
    Book old("old", "T", "A", 1);
    Book fresh("fresh", "T", "A", 1);
    overdueCard.borrow(old, student, Date(2026, 10, 1));
    EXPECT_TRUE(overdueCard.hasOverdue(today()));
    EXPECT_THROW(overdueCard.borrow(fresh, student, today()), LibraryException);
    EXPECT_EQ(overdueCard.totalFineOn(today(), Money(10)), Money(260));
}
