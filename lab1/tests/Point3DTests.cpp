/**
 * @file Point3DTests.cpp
 * @author Коледа Ксения
 * @brief Модульные тесты класса Point3D.
 */
#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

#include "Point3D.h"

using lab1::Point3D;

TEST(Point3DTest, DefaultConstructorGivesOrigin) {
    const Point3D origin;
    EXPECT_EQ(origin.getX(), 0.0);
    EXPECT_EQ(origin.getY(), 0.0);
    EXPECT_EQ(origin.getZ(), 0.0);
}

TEST(Point3DTest, ConstructorStoresCoordinates) {
    const Point3D point(1.5, -2.0, 3.0);
    EXPECT_EQ(point.getX(), 1.5);
    EXPECT_EQ(point.getY(), -2.0);
    EXPECT_EQ(point.getZ(), 3.0);
}

TEST(Point3DTest, ArithmeticOperators) {
    const Point3D first(1, 2, 3);
    const Point3D second(4, 5, 6);
    EXPECT_EQ(first + second, Point3D(5, 7, 9));
    EXPECT_EQ(second - first, Point3D(3, 3, 3));
    EXPECT_EQ(first * 2.0, Point3D(2, 4, 6));
    EXPECT_EQ(second / 2.0, Point3D(2, 2.5, 3));
}

TEST(Point3DTest, DivisionByZeroThrows) {
    EXPECT_THROW(Point3D(1, 1, 1) / 0.0, std::domain_error);
}

TEST(Point3DTest, DotAndCrossProducts) {
    const Point3D first(1, 2, 3);
    const Point3D second(4, 5, 6);
    EXPECT_EQ(first.dot(second), 32.0);
    EXPECT_EQ(first.cross(second), Point3D(-3, 6, -3));
}

TEST(Point3DTest, LengthUsesPythagoras) {
    const Point3D point(3, 4, 12);
    EXPECT_EQ(point.lengthSquared(), 169.0);
    EXPECT_EQ(point.length(), 13.0);
}

TEST(Point3DTest, EqualityUsesTolerance) {
    EXPECT_TRUE(Point3D(1, 2, 3) == Point3D(1 + 1e-12, 2, 3));
    EXPECT_TRUE(Point3D(1, 2, 3) != Point3D(1.001, 2, 3));
}

TEST(Point3DTest, StreamOutputFormat) {
    std::ostringstream out;
    out << Point3D(1, 2.5, -3);
    EXPECT_EQ(out.str(), "(1, 2.5, -3)");
}

TEST(Point3DTest, StreamInputParsesFormat) {
    std::istringstream in("  ( 1 , -2.5,3 ) ");
    Point3D point;
    in >> point;
    EXPECT_TRUE(static_cast<bool>(in));
    EXPECT_EQ(point, Point3D(1, -2.5, 3));
}

TEST(Point3DTest, StreamInputRejectsBadFormatAndKeepsValue) {
    std::istringstream in("(1; 2; 3)");
    Point3D point(7, 7, 7);
    in >> point;
    EXPECT_TRUE(in.fail());
    EXPECT_EQ(point, Point3D(7, 7, 7));
}
