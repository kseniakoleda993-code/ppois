/**
 * @file BigInteger.h
 * @author Коледа Ксения
 * @brief Знаковое целое число неограниченной длины.
 */
#ifndef LAB1_BIGINTEGER_H
#define LAB1_BIGINTEGER_H

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace lab1 {

/**
 * @brief Знаковое целое число неограниченной длины.
 * @details Число хранится как знак и модуль. Модуль - массив "цифр" в системе счисления
 * 10^9 (младшая цифра первая), ноль представлен пустым массивом и всегда неотрицателен.
 * Конструктор от long long не помечен explicit намеренно: благодаря этому все операции
 * автоматически работают и с обычными целыми (a + 5, 5 + a, a < 5).
 * Деление усекает результат к нулю, как и деление встроенных целых.
 * Копирование выполняется конструктором и оператором присваивания, сгенерированными
 * компилятором: std::vector сам корректно копирует и освобождает память (правило нуля).
 */
class BigInteger {
public:
    /// Создаёт число 0.
    BigInteger() = default;

    /// Создаёт число из встроенного целого (в том числе из LLONG_MIN).
    BigInteger(long long value);

    /**
     * @brief Создаёт число из строки вида "[+|-]цифры".
     * @param text десятичная запись числа
     * @throws std::invalid_argument если строка пуста или содержит недопустимые символы
     */
    explicit BigInteger(const std::string& text);

    /**
     * @brief Приведение к встроенному целому типу.
     * @return значение числа
     * @throws std::overflow_error если число не помещается в long long
     */
    explicit operator long long() const;

    /// @return десятичная запись числа
    std::string toString() const;
    /// @return true, если число меньше нуля
    bool isNegative() const;
    /// @return true, если число равно нулю
    bool isZero() const;

    /// @return отрицательное значение, 0 или положительное: результат сравнения с other
    int compareTo(const BigInteger& other) const;

    /// @return число с противоположным знаком
    BigInteger operator-() const;

    /// Сложение. @return ссылка на себя
    BigInteger& operator+=(const BigInteger& other);
    /// Вычитание. @return ссылка на себя
    BigInteger& operator-=(const BigInteger& other);
    /// Умножение. @return ссылка на себя
    BigInteger& operator*=(const BigInteger& other);

    /**
     * @brief Деление с усечением к нулю.
     * @param other делитель
     * @return ссылка на себя
     * @throws std::domain_error если делитель равен нулю
     */
    BigInteger& operator/=(const BigInteger& other);

    /// Префиксный инкремент. @return ссылка на себя
    BigInteger& operator++();
    /// Постфиксный инкремент. @return значение до увеличения
    BigInteger operator++(int);
    /// Префиксный декремент. @return ссылка на себя
    BigInteger& operator--();
    /// Постфиксный декремент. @return значение до уменьшения
    BigInteger operator--(int);

private:
    void normalize();

    bool negative_ = false;
    std::vector<std::uint32_t> digits_;
};

/// @return сумма
BigInteger operator+(BigInteger lhs, const BigInteger& rhs);
/// @return разность
BigInteger operator-(BigInteger lhs, const BigInteger& rhs);
/// @return произведение
BigInteger operator*(BigInteger lhs, const BigInteger& rhs);
/// @return частное (усечение к нулю)
/// @throws std::domain_error если делитель равен нулю
BigInteger operator/(BigInteger lhs, const BigInteger& rhs);

/// Равенство (работает и для сравнения с обычным целым).
bool operator==(const BigInteger& lhs, const BigInteger& rhs);
/// Неравенство.
bool operator!=(const BigInteger& lhs, const BigInteger& rhs);
/// Меньше.
bool operator<(const BigInteger& lhs, const BigInteger& rhs);
/// Меньше или равно.
bool operator<=(const BigInteger& lhs, const BigInteger& rhs);
/// Больше.
bool operator>(const BigInteger& lhs, const BigInteger& rhs);
/// Больше или равно.
bool operator>=(const BigInteger& lhs, const BigInteger& rhs);

/// Вывод в десятичной записи.
std::ostream& operator<<(std::ostream& out, const BigInteger& value);

/**
 * @brief Чтение числа из потока (одно слово).
 * @details При ошибке формата у потока выставляется failbit, а value не изменяется.
 */
std::istream& operator>>(std::istream& in, BigInteger& value);

}  // namespace lab1

#endif  // LAB1_BIGINTEGER_H
