#pragma once

#include <stdexcept>
#include <string>

namespace studis {

/// Базовое исключение информационной системы студентов.
class StudentSystemException : public std::runtime_error {
public:
    explicit StudentSystemException(const std::string& message) : std::runtime_error(message) {}
};

/// Некорректные данные студента или сотрудника.
class InvalidStudentDataException : public StudentSystemException {
public:
    explicit InvalidStudentDataException(const std::string& details)
        : StudentSystemException("Некорректные данные учащегося или сотрудника: " + details) {}
};

/// Оценка вне допустимого диапазона.
class InvalidGradeException : public StudentSystemException {
public:
    explicit InvalidGradeException(const std::string& details)
        : StudentSystemException("Недопустимая оценка: " + details) {}
};

/// Некорректные данные дисциплины или учебного плана.
class InvalidCourseDataException : public StudentSystemException {
public:
    explicit InvalidCourseDataException(const std::string& details)
        : StudentSystemException("Некорректные данные дисциплины: " + details) {}
};

/// В группе дисциплины нет свободных мест.
class CourseFullException : public StudentSystemException {
public:
    explicit CourseFullException(const std::string& details)
        : StudentSystemException("Нет свободных мест на дисциплине: " + details) {}
};

/// Не выполнены пререквизиты дисциплины.
class PrerequisiteNotMetException : public StudentSystemException {
public:
    explicit PrerequisiteNotMetException(const std::string& details)
        : StudentSystemException("Не выполнены пререквизиты: " + details) {}
};

/// Конфликт расписания или перегрузка преподавателя.
class ScheduleConflictException : public StudentSystemException {
public:
    explicit ScheduleConflictException(const std::string& details)
        : StudentSystemException("Конфликт расписания: " + details) {}
};

/// Некорректная дата или время.
class InvalidDateException : public StudentSystemException {
public:
    explicit InvalidDateException(const std::string& details)
        : StudentSystemException("Некорректная дата или время: " + details) {}
};

/// Недостаточно зачётных единиц.
class InsufficientCreditsException : public StudentSystemException {
public:
    explicit InsufficientCreditsException(const std::string& details)
        : StudentSystemException("Недостаточно зачётных единиц: " + details) {}
};

/// Повторная запись (зачисление, отметка, заселение).
class DuplicateRecordException : public StudentSystemException {
public:
    explicit DuplicateRecordException(const std::string& details)
        : StudentSystemException("Запись уже существует: " + details) {}
};

/// Некорректный платёж за обучение.
class TuitionPaymentException : public StudentSystemException {
public:
    explicit TuitionPaymentException(const std::string& details)
        : StudentSystemException("Ошибка оплаты обучения: " + details) {}
};

/// Недопустимая денежная операция.
class InvalidMoneyOperationException : public StudentSystemException {
public:
    explicit InvalidMoneyOperationException(const std::string& details)
        : StudentSystemException("Недопустимая денежная операция: " + details) {}
};

/// Студент не имеет права на стипендию.
class ScholarshipEligibilityException : public StudentSystemException {
public:
    explicit ScholarshipEligibilityException(const std::string& details)
        : StudentSystemException("Нет права на стипендию: " + details) {}
};

/// Операция недопустима в текущем состоянии экзамена или работы.
class InvalidExamStateException : public StudentSystemException {
public:
    explicit InvalidExamStateException(const std::string& details)
        : StudentSystemException("Недопустимое состояние экзамена: " + details) {}
};

/// Действие запрещено из-за академической задолженности.
class AcademicProbationException : public StudentSystemException {
public:
    explicit AcademicProbationException(const std::string& details)
        : StudentSystemException("Академическая задолженность: " + details) {}
};

/// Недостаточно мест в аудитории, группе или комнате.
class RoomCapacityException : public StudentSystemException {
public:
    explicit RoomCapacityException(const std::string& details)
        : StudentSystemException("Недостаточно мест: " + details) {}
};

/// Документ недействителен или не принадлежит студенту.
class DocumentException : public StudentSystemException {
public:
    explicit DocumentException(const std::string& details)
        : StudentSystemException("Проблема с документом: " + details) {}
};

/// Операция с библиотекой невозможна.
class LibraryException : public StudentSystemException {
public:
    explicit LibraryException(const std::string& details)
        : StudentSystemException("Ошибка библиотеки: " + details) {}
};

/// Поступление невозможно.
class AdmissionException : public StudentSystemException {
public:
    explicit AdmissionException(const std::string& details)
        : StudentSystemException("Ошибка поступления: " + details) {}
};

/// Запрошенный объект не найден.
class NotFoundException : public StudentSystemException {
public:
    explicit NotFoundException(const std::string& details)
        : StudentSystemException("Не найдено: " + details) {}
};

}  // namespace studis
