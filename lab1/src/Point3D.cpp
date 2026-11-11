/**
 * @file Point3D.cpp
 * @author Коледа Ксения
 * @brief Реализация класса Point3D.
 */
#include "Point3D.h"

#include <cmath>
#include <istream>
#include <ostream>
#include <stdexcept>

#include "StreamParsing.h"

namespace lab1 {

namespace {
bool areClose(double lhs, double rhs) {
    return std::fabs(lhs - rhs) <= kCoordinateEpsilon;
}
}  // namespace

Point3D::Point3D(double x, double y, double z) : x_(x), y_(y), z_(z) {}

double Point3D::getX() const { return x_; }
double Point3D::getY() const { return y_; }
double Point3D::getZ() const { return z_; }

Point3D& Point3D::operator+=(const Point3D& other) {
    x_ += other.x_;
    y_ += other.y_;
    z_ += other.z_;
    return *this;
}

Point3D& Point3D::operator-=(const Point3D& other) {
    x_ -= other.x_;
    y_ -= other.y_;
    z_ -= other.z_;
    return *this;
}

Point3D& Point3D::operator*=(double factor) {
    x_ *= factor;
    y_ *= factor;
    z_ *= factor;
    return *this;
}

Point3D& Point3D::operator/=(double divisor) {
    if (divisor == 0.0) {
        throw std::domain_error("Деление на ноль");
    }
    x_ /= divisor;
    y_ /= divisor;
    z_ /= divisor;
    return *this;
}

double Point3D::dot(const Point3D& other) const {
    return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
}

Point3D Point3D::cross(const Point3D& other) const {
    return Point3D(y_ * other.z_ - z_ * other.y_,
                   z_ * other.x_ - x_ * other.z_,
                   x_ * other.y_ - y_ * other.x_);
}

double Point3D::lengthSquared() const { return dot(*this); }

double Point3D::length() const { return std::sqrt(lengthSquared()); }

Point3D operator+(Point3D lhs, const Point3D& rhs) { return lhs += rhs; }
Point3D operator-(Point3D lhs, const Point3D& rhs) { return lhs -= rhs; }
Point3D operator*(Point3D point, double factor) { return point *= factor; }
Point3D operator/(Point3D point, double divisor) { return point /= divisor; }

bool operator==(const Point3D& lhs, const Point3D& rhs) {
    return areClose(lhs.getX(), rhs.getX()) && areClose(lhs.getY(), rhs.getY()) &&
           areClose(lhs.getZ(), rhs.getZ());
}

bool operator!=(const Point3D& lhs, const Point3D& rhs) { return !(lhs == rhs); }

std::ostream& operator<<(std::ostream& out, const Point3D& point) {
    return out << '(' << point.getX() << ", " << point.getY() << ", " << point.getZ() << ')';
}

std::istream& operator>>(std::istream& in, Point3D& point) {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    const bool parsed = expectSymbol(in, '(') && (in >> x) && expectSymbol(in, ',') &&
                        (in >> y) && expectSymbol(in, ',') && (in >> z) && expectSymbol(in, ')');
    if (parsed) {
        point = Point3D(x, y, z);
    }
    return in;
}

}  // namespace lab1
