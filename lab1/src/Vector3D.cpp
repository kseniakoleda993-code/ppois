/**
 * @file Vector3D.cpp
 * @author Коледа Ксения
 * @brief Реализация класса Vector3D.
 */
#include "Vector3D.h"

#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>
#include <stdexcept>

#include "StreamParsing.h"

namespace lab1 {

namespace {
constexpr double kMinCosine = -1.0;
constexpr double kMaxCosine = 1.0;
}  // namespace

Vector3D::Vector3D(const Point3D& begin, const Point3D& end) : begin_(begin), end_(end) {}

const Point3D& Vector3D::getBegin() const { return begin_; }
const Point3D& Vector3D::getEnd() const { return end_; }

Point3D Vector3D::getDirection() const { return end_ - begin_; }

double Vector3D::length() const { return getDirection().length(); }

double Vector3D::cosineWith(const Vector3D& other) const {
    const double lengthsProduct = length() * other.length();
    if (length() <= kCoordinateEpsilon || other.length() <= kCoordinateEpsilon) {
        throw std::domain_error("Косинус угла с нулевым вектором не определён");
    }
    const double cosine = getDirection().dot(other.getDirection()) / lengthsProduct;
    return std::clamp(cosine, kMinCosine, kMaxCosine);
}

bool Vector3D::hasSameLengthAs(const Vector3D& other) const {
    return std::fabs(length() - other.length()) <= kCoordinateEpsilon;
}

void Vector3D::assignDirection(const Point3D& direction) { end_ = begin_ + direction; }

Vector3D& Vector3D::operator+=(const Vector3D& other) {
    assignDirection(getDirection() + other.getDirection());
    return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& other) {
    assignDirection(getDirection() - other.getDirection());
    return *this;
}

Vector3D& Vector3D::operator*=(const Vector3D& other) {
    assignDirection(getDirection().cross(other.getDirection()));
    return *this;
}

Vector3D& Vector3D::operator*=(double factor) {
    assignDirection(getDirection() * factor);
    return *this;
}

Vector3D& Vector3D::operator/=(double divisor) {
    assignDirection(getDirection() / divisor);
    return *this;
}

Vector3D operator+(Vector3D lhs, const Vector3D& rhs) { return lhs += rhs; }
Vector3D operator-(Vector3D lhs, const Vector3D& rhs) { return lhs -= rhs; }
Vector3D operator*(Vector3D lhs, const Vector3D& rhs) { return lhs *= rhs; }
Vector3D operator*(Vector3D vector, double factor) { return vector *= factor; }
Vector3D operator*(double factor, Vector3D vector) { return vector *= factor; }
Vector3D operator/(Vector3D vector, double divisor) { return vector /= divisor; }

double operator^(const Vector3D& lhs, const Vector3D& rhs) { return lhs.cosineWith(rhs); }

bool operator==(const Vector3D& lhs, const Vector3D& rhs) {
    return lhs.getBegin() == rhs.getBegin() && lhs.getEnd() == rhs.getEnd();
}

bool operator!=(const Vector3D& lhs, const Vector3D& rhs) { return !(lhs == rhs); }

bool operator<(const Vector3D& lhs, const Vector3D& rhs) {
    return lhs.length() < rhs.length() && !lhs.hasSameLengthAs(rhs);
}

bool operator<=(const Vector3D& lhs, const Vector3D& rhs) {
    return lhs < rhs || lhs.hasSameLengthAs(rhs);
}

bool operator>(const Vector3D& lhs, const Vector3D& rhs) { return rhs < lhs; }
bool operator>=(const Vector3D& lhs, const Vector3D& rhs) { return rhs <= lhs; }

std::ostream& operator<<(std::ostream& out, const Vector3D& vector) {
    return out << '[' << vector.getBegin() << ", " << vector.getEnd() << ']';
}

std::istream& operator>>(std::istream& in, Vector3D& vector) {
    Point3D begin;
    Point3D end;
    const bool parsed = expectSymbol(in, '[') && (in >> begin) && expectSymbol(in, ',') &&
                        (in >> end) && expectSymbol(in, ']');
    if (parsed) {
        vector = Vector3D(begin, end);
    }
    return in;
}

}  // namespace lab1
