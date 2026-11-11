/**
 * @file Point3D.h
 * @author Коледа Ксения
 * @brief Точка (набор из трёх координат) в трёхмерном пространстве.
 */
#ifndef LAB1_POINT3D_H
#define LAB1_POINT3D_H

#include <iosfwd>

namespace lab1 {

/// Допустимая погрешность при сравнении вещественных чисел.
constexpr double kCoordinateEpsilon = 1e-9;

/**
 * @brief Тройка вещественных координат (x, y, z).
 * @details Используется и как точка пространства, и как смещение (разность двух точек).
 * Все поля скрыты (инкапсуляция), доступ осуществляется через методы.
 * @see Vector3D
 */
class Point3D {
public:
    /// Создаёт точку в начале координат.
    Point3D() = default;

    /**
     * @brief Создаёт точку по координатам.
     * @param x координата по оси X
     * @param y координата по оси Y
     * @param z координата по оси Z
     */
    Point3D(double x, double y, double z);

    /// @return координата по оси X
    double getX() const;
    /// @return координата по оси Y
    double getY() const;
    /// @return координата по оси Z
    double getZ() const;

    /// Покоординатное сложение. @return ссылка на себя
    Point3D& operator+=(const Point3D& other);
    /// Покоординатное вычитание. @return ссылка на себя
    Point3D& operator-=(const Point3D& other);
    /// Умножение всех координат на число. @return ссылка на себя
    Point3D& operator*=(double factor);

    /**
     * @brief Деление всех координат на число.
     * @param divisor делитель
     * @return ссылка на себя
     * @throws std::domain_error если делитель равен нулю
     */
    Point3D& operator/=(double divisor);

    /// @return скалярное произведение с другой точкой
    double dot(const Point3D& other) const;
    /// @return векторное произведение с другой точкой
    Point3D cross(const Point3D& other) const;
    /// @return квадрат расстояния до начала координат
    double lengthSquared() const;
    /// @return расстояние до начала координат
    double length() const;

private:
    double x_ = 0.0;
    double y_ = 0.0;
    double z_ = 0.0;
};

/// @return покоординатная сумма
Point3D operator+(Point3D lhs, const Point3D& rhs);
/// @return покоординатная разность
Point3D operator-(Point3D lhs, const Point3D& rhs);
/// @return точка, умноженная на число
Point3D operator*(Point3D point, double factor);
/// @return точка, делённая на число
/// @throws std::domain_error если делитель равен нулю
Point3D operator/(Point3D point, double divisor);

/// Сравнение координат с точностью kCoordinateEpsilon.
bool operator==(const Point3D& lhs, const Point3D& rhs);
/// Отрицание operator==.
bool operator!=(const Point3D& lhs, const Point3D& rhs);

/// Вывод в формате "(x, y, z)".
std::ostream& operator<<(std::ostream& out, const Point3D& point);

/**
 * @brief Чтение из формата "(x, y, z)".
 * @details При ошибке формата у потока выставляется failbit, а point не изменяется.
 */
std::istream& operator>>(std::istream& in, Point3D& point);

}  // namespace lab1

#endif  // LAB1_POINT3D_H
