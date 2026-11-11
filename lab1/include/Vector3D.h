/**
 * @file Vector3D.h
 * @author Коледа Ксения
 * @brief Вектор в трёхмерном пространстве, заданный координатами начала и конца.
 */
#ifndef LAB1_VECTOR3D_H
#define LAB1_VECTOR3D_H

#include <iosfwd>

#include "Point3D.h"

namespace lab1 {

/**
 * @brief Вектор, заданный координатами концов (начало и конец) в трёхмерном пространстве.
 * @details Результат любой операции над векторами сохраняет начало первого операнда,
 * а конец вычисляется как начало + результирующее смещение.
 * Операции сравнения <, <=, >, >= сравнивают векторы по длине, а == и != - по координатам концов.
 * Копирование выполняется конструктором и оператором присваивания, сгенерированными компилятором,
 * так как класс хранит только значения и не владеет внешними ресурсами.
 * @see Point3D
 */
class Vector3D {
public:
    /// Создаёт нулевой вектор в начале координат.
    Vector3D() = default;

    /**
     * @brief Создаёт вектор по координатам концов.
     * @param begin координаты начала
     * @param end координаты конца
     */
    Vector3D(const Point3D& begin, const Point3D& end);

    /// @return координаты начала вектора
    const Point3D& getBegin() const;
    /// @return координаты конца вектора
    const Point3D& getEnd() const;
    /// @return смещение (конец минус начало)
    Point3D getDirection() const;
    /// @return длина вектора
    double length() const;

    /**
     * @brief Косинус угла между векторами.
     * @param other второй вектор
     * @return значение косинуса в диапазоне [-1; 1]
     * @throws std::domain_error если хотя бы один из векторов нулевой
     */
    double cosineWith(const Vector3D& other) const;

    /// @return true, если длины векторов совпадают с точностью kCoordinateEpsilon
    bool hasSameLengthAs(const Vector3D& other) const;

    /// Сложение векторов. @return ссылка на себя
    Vector3D& operator+=(const Vector3D& other);
    /// Вычитание векторов. @return ссылка на себя
    Vector3D& operator-=(const Vector3D& other);
    /// Векторное произведение. @return ссылка на себя
    Vector3D& operator*=(const Vector3D& other);
    /// Умножение на число. @return ссылка на себя
    Vector3D& operator*=(double factor);

    /**
     * @brief Деление на число.
     * @param divisor делитель
     * @return ссылка на себя
     * @throws std::domain_error если делитель равен нулю
     */
    Vector3D& operator/=(double divisor);

private:
    void assignDirection(const Point3D& direction);

    Point3D begin_;
    Point3D end_;
};

/// @return сумма векторов
Vector3D operator+(Vector3D lhs, const Vector3D& rhs);
/// @return разность векторов
Vector3D operator-(Vector3D lhs, const Vector3D& rhs);
/// @return векторное произведение
Vector3D operator*(Vector3D lhs, const Vector3D& rhs);
/// @return вектор, умноженный на число
Vector3D operator*(Vector3D vector, double factor);
/// @return вектор, умноженный на число (число слева)
Vector3D operator*(double factor, Vector3D vector);
/// @return вектор, делённый на число
/// @throws std::domain_error если делитель равен нулю
Vector3D operator/(Vector3D vector, double divisor);

/**
 * @brief Косинус угла между векторами (оператор ^).
 * @details Приоритет ^ ниже, чем у арифметических операторов, поэтому выражение нужно брать в скобки.
 * @see Vector3D::cosineWith
 */
double operator^(const Vector3D& lhs, const Vector3D& rhs);

/// Равенство по координатам начала и конца.
bool operator==(const Vector3D& lhs, const Vector3D& rhs);
/// Неравенство по координатам начала и конца.
bool operator!=(const Vector3D& lhs, const Vector3D& rhs);
/// Сравнение по длине: lhs короче rhs.
bool operator<(const Vector3D& lhs, const Vector3D& rhs);
/// Сравнение по длине: lhs не длиннее rhs.
bool operator<=(const Vector3D& lhs, const Vector3D& rhs);
/// Сравнение по длине: lhs длиннее rhs.
bool operator>(const Vector3D& lhs, const Vector3D& rhs);
/// Сравнение по длине: lhs не короче rhs.
bool operator>=(const Vector3D& lhs, const Vector3D& rhs);

/// Вывод в формате "[(x, y, z), (x, y, z)]" (начало, конец).
std::ostream& operator<<(std::ostream& out, const Vector3D& vector);

/**
 * @brief Чтение из формата "[(x, y, z), (x, y, z)]".
 * @details При ошибке формата у потока выставляется failbit, а vector не изменяется.
 */
std::istream& operator>>(std::istream& in, Vector3D& vector);

}  // namespace lab1

#endif  // LAB1_VECTOR3D_H
