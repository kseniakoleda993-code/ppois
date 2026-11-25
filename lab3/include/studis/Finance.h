#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "Grades.h"
#include "People.h"
#include "Structure.h"

namespace studis {

inline constexpr int kDaysPerMonth = 30;
inline constexpr double kHonorsBonusPercent = 20.0;
inline constexpr const char* kOnlineMethod = "online";
inline constexpr const char* kCashMethod = "cash";
inline constexpr const char* kBankMethod = "bank";

/// Договор об оплате обучения.
class TuitionContract {
public:
    TuitionContract(int id, const Student& student, const StudyProgram& program, const Money& totalFee, int installments)
        : id_(id), studentId_(student.getId()), programCode_(program.getCode()), totalFee_(totalFee),
          installments_(installments) {
        if (totalFee_.isNegative() || totalFee_.isZero() || installments_ <= 0) {
            throw TuitionPaymentException("некорректный договор №" + std::to_string(id_));
        }
    }

    int getId() const { return id_; }
    int getStudentId() const { return studentId_; }
    const std::string& getProgramCode() const { return programCode_; }
    const Money& getTotalFee() const { return totalFee_; }
    const Money& getPaid() const { return paid_; }
    int getInstallments() const { return installments_; }
    Money outstanding() const { return totalFee_.minus(paid_); }
    bool isFullyPaid() const { return outstanding().isZero(); }
    Money installmentAmount() const { return totalFee_.multiply(1.0 / installments_); }
    int paidInstallments() const {
        return static_cast<int>(paid_.getCents() / installmentAmount().getCents());
    }
    void registerPayment(const Money& amount) {
        if (amount.isNegative() || amount.isZero() || !outstanding().isGreaterOrEqual(amount)) {
            throw TuitionPaymentException("сумма платежа " + amount.toString());
        }
        paid_ = paid_.plus(amount);
    }

private:
    int id_;
    int studentId_;
    std::string programCode_;
    Money totalFee_;
    Money paid_;
    int installments_;
};

/// Платёж за обучение.
class TuitionPayment {
public:
    TuitionPayment(int id, const TuitionContract& contract, const Money& amount, const Date& paidOn, std::string method)
        : id_(id), contractId_(contract.getId()), amount_(amount), paidOn_(paidOn), method_(std::move(method)) {
        if (amount_.isNegative() || amount_.isZero()) {
            throw TuitionPaymentException("платёж должен быть положительным");
        }
        if (method_ != kOnlineMethod && method_ != kCashMethod && method_ != kBankMethod) {
            throw TuitionPaymentException("способ оплаты " + method_);
        }
    }

    int getId() const { return id_; }
    int getContractId() const { return contractId_; }
    const Money& getAmount() const { return amount_; }
    const Date& getPaidOn() const { return paidOn_; }
    const std::string& getMethod() const { return method_; }
    bool isOnline() const { return method_ == kOnlineMethod; }
    bool isLate(const Date& dueDate) const { return dueDate.isBefore(paidOn_); }
    std::string receiptText() const {
        return "Платёж №" + std::to_string(id_) + " по договору №" + std::to_string(contractId_) + ": " +
               amount_.toString() + " (" + method_ + ")";
    }

private:
    int id_;
    int contractId_;
    Money amount_;
    Date paidOn_;
    std::string method_;
};

/// Вид стипендии.
class Scholarship {
public:
    Scholarship(std::string name, const Money& monthlyAmount, double minGpa, int durationMonths)
        : name_(std::move(name)), monthlyAmount_(monthlyAmount), minGpa_(minGpa), durationMonths_(durationMonths) {}

    const std::string& getName() const { return name_; }
    const Money& getMonthlyAmount() const { return monthlyAmount_; }
    double getMinGpa() const { return minGpa_; }
    int getDurationMonths() const { return durationMonths_; }
    bool isEligible(const RecordBook& recordBook) const {
        return !recordBook.getEntries().empty() && recordBook.gpa() >= minGpa_ && recordBook.failedCourses().empty();
    }
    Money totalValue() const { return monthlyAmount_.multiply(durationMonths_); }
    Money amountFor(const RecordBook& recordBook) const {
        if (!recordBook.hasHonors()) {
            return monthlyAmount_;
        }
        return monthlyAmount_.plus(monthlyAmount_.applyPercent(kHonorsBonusPercent));
    }

private:
    std::string name_;
    Money monthlyAmount_;
    double minGpa_;
    int durationMonths_;
};

/// Назначение стипендии студенту.
class ScholarshipAward {
public:
    ScholarshipAward(const Student& student, const Scholarship& scholarship, const RecordBook& recordBook,
                     const Date& startDate)
        : studentId_(student.getId()), scholarshipName_(scholarship.getName()),
          monthlyAmount_(scholarship.amountFor(recordBook)), startDate_(startDate),
          endDate_(startDate.plusDays(scholarship.getDurationMonths() * kDaysPerMonth)),
          months_(scholarship.getDurationMonths()) {
        if (!student.isActive() || !scholarship.isEligible(recordBook)) {
            throw ScholarshipEligibilityException(student.getName());
        }
    }

    int getStudentId() const { return studentId_; }
    const std::string& getScholarshipName() const { return scholarshipName_; }
    const Money& getMonthlyAmount() const { return monthlyAmount_; }
    const Date& getStartDate() const { return startDate_; }
    const Date& getEndDate() const { return endDate_; }
    bool isRevoked() const { return revoked_; }
    void revoke() { revoked_ = true; }
    bool isActiveOn(const Date& date) const {
        return !revoked_ && !date.isBefore(startDate_) && !endDate_.isBefore(date);
    }
    int monthsLeft(const Date& date) const {
        if (revoked_ || endDate_.isBefore(date)) {
            return 0;
        }
        return std::max(0, months_ - std::max(0, startDate_.daysUntil(date)) / kDaysPerMonth);
    }
    Money totalPaidUntil(const Date& date) const {
        const int elapsed = std::min(months_, std::max(0, startDate_.daysUntil(date)) / kDaysPerMonth);
        return monthlyAmount_.multiply(elapsed);
    }

private:
    int studentId_;
    std::string scholarshipName_;
    Money monthlyAmount_;
    Date startDate_;
    Date endDate_;
    int months_;
    bool revoked_ = false;
};

/// Комната общежития.
class DormitoryRoom {
public:
    DormitoryRoom(int number, int capacity) : number_(number), capacity_(capacity) {
        if (capacity_ <= 0) {
            throw RoomCapacityException("вместимость комнаты " + std::to_string(number_));
        }
    }

    int getNumber() const { return number_; }
    int getCapacity() const { return capacity_; }
    const std::vector<int>& getResidentIds() const { return residentIds_; }
    int freePlaces() const { return capacity_ - static_cast<int>(residentIds_.size()); }
    bool isFull() const { return freePlaces() <= 0; }
    bool hosts(const Student& student) const {
        return std::find(residentIds_.begin(), residentIds_.end(), student.getId()) != residentIds_.end();
    }
    void assign(const Student& student) {
        if (hosts(student)) {
            throw DuplicateRecordException(student.getName() + " уже проживает в комнате");
        }
        if (isFull()) {
            throw RoomCapacityException("комната " + std::to_string(number_) + " заполнена");
        }
        residentIds_.push_back(student.getId());
    }
    void vacate(const Student& student) {
        residentIds_.erase(std::remove(residentIds_.begin(), residentIds_.end(), student.getId()), residentIds_.end());
    }

private:
    int number_;
    int capacity_;
    std::vector<int> residentIds_;
};

/// Студенческое общежитие.
class Dormitory {
public:
    Dormitory(std::string name, const Address& address, const Money& monthlyFee)
        : name_(std::move(name)), address_(address), monthlyFee_(monthlyFee) {}

    const std::string& getName() const { return name_; }
    const Address& getAddress() const { return address_; }
    const Money& getMonthlyFee() const { return monthlyFee_; }
    const std::vector<DormitoryRoom>& getRooms() const { return rooms_; }
    void addRoom(const DormitoryRoom& room) { rooms_.push_back(room); }
    int settle(const Student& student) {
        if (student.isOnProbation()) {
            throw AcademicProbationException(student.getName());
        }
        if (!student.isActive()) {
            throw InvalidStudentDataException("заселение неактивного студента " + student.getName());
        }
        for (DormitoryRoom& room : rooms_) {
            if (!room.isFull()) {
                room.assign(student);
                return room.getNumber();
            }
        }
        throw RoomCapacityException("в общежитии " + name_ + " нет мест");
    }
    void evict(const Student& student) {
        for (DormitoryRoom& room : rooms_) {
            room.vacate(student);
        }
    }
    int totalFreePlaces() const {
        int total = 0;
        for (const DormitoryRoom& room : rooms_) {
            total += room.freePlaces();
        }
        return total;
    }
    double occupancyPercent() const {
        int capacity = 0;
        for (const DormitoryRoom& room : rooms_) {
            capacity += room.getCapacity();
        }
        return capacity == 0 ? 0.0 : kPercentBase * (capacity - totalFreePlaces()) / capacity;
    }
    Money feeForMonths(int months) const { return monthlyFee_.multiply(months); }

private:
    std::string name_;
    Address address_;
    Money monthlyFee_;
    std::vector<DormitoryRoom> rooms_;
};

/// Книга библиотеки.
class Book {
public:
    Book(std::string isbn, std::string title, std::string author, int copies)
        : isbn_(std::move(isbn)), title_(std::move(title)), author_(std::move(author)), totalCopies_(copies),
          availableCopies_(copies) {
        if (isbn_.empty() || totalCopies_ <= 0) {
            throw LibraryException("книга " + isbn_);
        }
    }

    const std::string& getIsbn() const { return isbn_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getAuthor() const { return author_; }
    int getTotalCopies() const { return totalCopies_; }
    int getAvailableCopies() const { return availableCopies_; }
    bool isAvailable() const { return availableCopies_ > 0; }
    int borrowedCount() const { return totalCopies_ - availableCopies_; }
    void lend() {
        if (!isAvailable()) {
            throw LibraryException("нет свободных экземпляров: " + title_);
        }
        --availableCopies_;
    }
    void giveBack() {
        if (availableCopies_ >= totalCopies_) {
            throw LibraryException("все экземпляры уже в библиотеке: " + title_);
        }
        ++availableCopies_;
    }

private:
    std::string isbn_;
    std::string title_;
    std::string author_;
    int totalCopies_;
    int availableCopies_;
};

inline constexpr int kMaxLoanExtensions = 2;
inline constexpr int kMaxActiveLoans = 5;
inline constexpr int kDefaultLoanDays = 14;

/// Выдача книги студенту.
class BookLoan {
public:
    BookLoan(const Book& book, const Student& student, const Date& borrowedOn, int days)
        : isbn_(book.getIsbn()), studentId_(student.getId()), borrowedOn_(borrowedOn),
          dueOn_(borrowedOn.plusDays(days)) {}

    const std::string& getIsbn() const { return isbn_; }
    int getStudentId() const { return studentId_; }
    const Date& getBorrowedOn() const { return borrowedOn_; }
    const Date& getDueOn() const { return dueOn_; }
    bool isReturned() const { return returned_; }
    int getExtensions() const { return extensions_; }
    bool isOverdueOn(const Date& date) const { return !returned_ && dueOn_.isBefore(date); }
    int daysOverdue(const Date& date) const { return isOverdueOn(date) ? dueOn_.daysUntil(date) : 0; }
    Money fineOn(const Date& date, const Money& perDay) const { return perDay.multiply(daysOverdue(date)); }
    void markReturned() { returned_ = true; }
    void extend(int days) {
        if (returned_ || extensions_ >= kMaxLoanExtensions) {
            throw LibraryException("продление невозможно: " + isbn_);
        }
        dueOn_ = dueOn_.plusDays(days);
        ++extensions_;
    }

private:
    std::string isbn_;
    int studentId_;
    Date borrowedOn_;
    Date dueOn_;
    bool returned_ = false;
    int extensions_ = 0;
};

/// Читательский билет студента.
class LibraryCard {
public:
    LibraryCard(const Student& student, const Date& issuedOn) : studentId_(student.getId()), issuedOn_(issuedOn) {}

    int getStudentId() const { return studentId_; }
    const Date& getIssuedOn() const { return issuedOn_; }
    const std::vector<BookLoan>& getLoans() const { return loans_; }
    int activeLoans() const {
        return static_cast<int>(std::count_if(loans_.begin(), loans_.end(), [](const BookLoan& l) { return !l.isReturned(); }));
    }
    bool hasOverdue(const Date& date) const {
        return std::any_of(loans_.begin(), loans_.end(), [&](const BookLoan& l) { return l.isOverdueOn(date); });
    }
    void borrow(Book& book, const Student& student, const Date& date) {
        if (activeLoans() >= kMaxActiveLoans) {
            throw LibraryException("превышен лимит выданных книг");
        }
        if (hasOverdue(date)) {
            throw LibraryException("есть просроченные книги");
        }
        book.lend();
        loans_.emplace_back(book, student, date, kDefaultLoanDays);
    }
    void giveBack(Book& book) {
        for (BookLoan& loan : loans_) {
            if (!loan.isReturned() && loan.getIsbn() == book.getIsbn()) {
                book.giveBack();
                loan.markReturned();
                return;
            }
        }
        throw NotFoundException("выдача книги " + book.getIsbn());
    }
    Money totalFineOn(const Date& date, const Money& perDay) const {
        Money total;
        for (const BookLoan& loan : loans_) {
            total = total.plus(loan.fineOn(date, perDay));
        }
        return total;
    }

private:
    int studentId_;
    Date issuedOn_;
    std::vector<BookLoan> loans_;
};

}  // namespace studis
