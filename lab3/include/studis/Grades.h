#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Common.h"
#include "People.h"

namespace studis {

inline constexpr int kMaxPoints = 100;
inline constexpr int kPassingPoints = 60;
inline constexpr int kSatisfactoryPoints = 70;
inline constexpr int kVeryGoodPoints = 80;
inline constexpr int kGoodPoints = 75;
inline constexpr int kExcellentPoints = 90;
inline constexpr int kExcellentFive = 5;
inline constexpr int kGoodFive = 4;
inline constexpr int kPassingFive = 3;
inline constexpr int kFailingFive = 2;

/// Оценка по 100-балльной шкале.
class Grade {
public:
    explicit Grade(int points = 0) : points_(points) {
        if (points_ < 0 || points_ > kMaxPoints) {
            throw InvalidGradeException(std::to_string(points_));
        }
    }

    int getPoints() const { return points_; }
    bool isPassing() const { return points_ >= kPassingPoints; }
    int toFivePoint() const {
        if (points_ >= kExcellentPoints) {
            return kExcellentFive;
        }
        if (points_ >= kGoodPoints) {
            return kGoodFive;
        }
        return isPassing() ? kPassingFive : kFailingFive;
    }
    std::string letter() const {
        if (points_ >= kExcellentPoints) {
            return "A";
        }
        if (points_ >= kVeryGoodPoints) {
            return "B";
        }
        if (points_ >= kSatisfactoryPoints) {
            return "C";
        }
        return isPassing() ? "D" : "F";
    }

private:
    int points_;
};

/// Запись в зачётной книжке: дисциплина, оценка и попытка.
class RecordEntry {
public:
    RecordEntry(std::string courseCode, int credits, const Grade& grade, const Date& takenOn, int attempt)
        : courseCode_(std::move(courseCode)), credits_(credits), grade_(grade), takenOn_(takenOn), attempt_(attempt) {}

    const std::string& getCourseCode() const { return courseCode_; }
    int getCredits() const { return credits_; }
    const Grade& getGrade() const { return grade_; }
    const Date& getTakenOn() const { return takenOn_; }
    int getAttempt() const { return attempt_; }
    bool isPassed() const { return grade_.isPassing(); }
    long long weightedPoints() const { return static_cast<long long>(credits_) * grade_.getPoints(); }

private:
    std::string courseCode_;
    int credits_;
    Grade grade_;
    Date takenOn_;
    int attempt_;
};

/// Зачётная книжка студента.
class RecordBook {
public:
    explicit RecordBook(const Student& student) : studentId_(student.getId()) {}

    int getStudentId() const { return studentId_; }
    const std::vector<RecordEntry>& getEntries() const { return entries_; }
    void addEntry(const RecordEntry& entry) {
        auto existing = std::find_if(entries_.begin(), entries_.end(), [&](const RecordEntry& e) {
            return e.getCourseCode() == entry.getCourseCode();
        });
        if (existing == entries_.end()) {
            entries_.push_back(entry);
        } else if (entry.getGrade().getPoints() >= existing->getGrade().getPoints()) {
            *existing = entry;
        }
    }
    bool hasPassed(const std::string& courseCode, int minPoints) const {
        return std::any_of(entries_.begin(), entries_.end(), [&](const RecordEntry& e) {
            return e.getCourseCode() == courseCode && e.getGrade().getPoints() >= minPoints && e.isPassed();
        });
    }
    int bestScore(const std::string& courseCode) const {
        for (const RecordEntry& entry : entries_) {
            if (entry.getCourseCode() == courseCode) {
                return entry.getGrade().getPoints();
            }
        }
        throw NotFoundException("оценка по дисциплине " + courseCode);
    }
    int earnedCredits() const {
        int total = 0;
        for (const RecordEntry& entry : entries_) {
            total += entry.isPassed() ? entry.getCredits() : 0;
        }
        return total;
    }
    double gpa() const {
        long long weighted = 0;
        long long credits = 0;
        for (const RecordEntry& entry : entries_) {
            weighted += entry.weightedPoints();
            credits += entry.getCredits();
        }
        return credits == 0 ? 0.0 : static_cast<double>(weighted) / static_cast<double>(credits);
    }
    std::vector<std::string> failedCourses() const {
        std::vector<std::string> codes;
        for (const RecordEntry& entry : entries_) {
            if (!entry.isPassed()) {
                codes.push_back(entry.getCourseCode());
            }
        }
        return codes;
    }
    bool hasHonors() const { return !entries_.empty() && failedCourses().empty() && gpa() >= kExcellentPoints; }

private:
    int studentId_;
    std::vector<RecordEntry> entries_;
};

}  // namespace studis
