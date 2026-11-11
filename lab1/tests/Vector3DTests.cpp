/**
 * @file Vector3DTests.cpp
 * @author Коледа Ксения
 * @brief Модульные тесты класса Vector3D.
 */
#include <gtest/gtest.h>

#include <cmath>
#include <sstream>
#include <stdexcept>

#include "Vector3D.h"

using lab1::Point3D;
using lab1::Vector3D;

namespace {
Vector3D fromOrigin(double x, double y, double z) {
    return Vector3D(Point3D(0, 0, 0), Point3D(x, y, z));
}
}  // namespace

TEST(Vector3DTest, DefaultVectorIsZero) {
    const Vector3D vector;
    EXPECT_EQ(vector.getBegin(), Point3D(0, 0, 0));
    EXPECT_EQ(vector.getEnd(), Point3D(0, 0, 0));
    EXPECT_EQ(vector.length(), 0.0);
}

TEST(Vector3DTest, GettersReturnEnds) {
    const Vector3D vector(Point3D(1, 2, 3), Point3D(4, 6, 3));
    EXPECT_EQ(vector.getBegin(), Point3D(1, 2, 3));
    EXPECT_EQ(vector.getEnd(), Point3D(4, 6, 3));
    EXPECT_EQ(vector.getDirection(), Point3D(3, 4, 0));
}

TEST(Vector3DTest, LengthIsComputedFromEnds) {
    const Vector3D vector(Point3D(1, 2, 3), Point3D(4, 6, 3));
    EXPECT_EQ(vector.length(), 5.0);
}

TEST(Vector3DTest, AdditionKeepsBeginOfLeftOperand) {
    const Vector3D left = fromOrigin(1, 2, 3);
    const Vector3D right(Point3D(1, 1, 1), Point3D(2, 3, 1));
    EXPECT_EQ(left + right, fromOrigin(2, 4, 3));
}

TEST(Vector3DTest, AdditionDoesNotChangeOperands) {
    const Vector3D left = fromOrigin(1, 0, 0);
    const Vector3D right = fromOrigin(0, 1, 0);
    const Vector3D sum = left + right;
    EXPECT_EQ(left, fromOrigin(1, 0, 0));
    EXPECT_EQ(right, fromOrigin(0, 1, 0));
    EXPECT_EQ(sum, fromOrigin(1, 1, 0));
}

TEST(Vector3DTest, CompoundAdditionReturnsReferenceToSelf) {
    Vector3D left = fromOrigin(1, 1, 1);
    Vector3D& result = (left += fromOrigin(1, 2, 3));
    EXPECT_TRUE(&result == &left);
    EXPECT_EQ(left, fromOrigin(2, 3, 4));
}

TEST(Vector3DTest, SelfAdditionDoublesVector) {
    Vector3D vector = fromOrigin(1, 2, 3);
    vector += vector;
    EXPECT_EQ(vector, fromOrigin(2, 4, 6));
}

TEST(Vector3DTest, SubtractionAndCompoundSubtraction) {
    const Vector3D left = fromOrigin(1, 2, 3);
    const Vector3D right(Point3D(1, 1, 1), Point3D(2, 3, 1));
    EXPECT_EQ(left - right, fromOrigin(0, 0, 3));
    Vector3D copy = left;
    copy -= right;
    EXPECT_EQ(copy, fromOrigin(0, 0, 3));
}

TEST(Vector3DTest, CrossProduct) {
    const Vector3D left = fromOrigin(1, 2, 3);
    const Vector3D right = fromOrigin(4, 5, 6);
    EXPECT_EQ(left * right, fromOrigin(-3, 6, -3));
}

TEST(Vector3DTest, CrossProductIsAntiCommutative) {
    const Vector3D left = fromOrigin(1, 2, 3);
    const Vector3D right = fromOrigin(4, 5, 6);
    EXPECT_EQ(left * right, (right * left) * -1.0);
}

TEST(Vector3DTest, CrossProductOfParallelVectorsIsZero) {
    const Vector3D vector = fromOrigin(1, 2, 3);
    EXPECT_EQ((vector * (vector * 2.0)).length(), 0.0);
}

TEST(Vector3DTest, CompoundCrossProduct) {
    Vector3D vector = fromOrigin(1, 0, 0);
    vector *= fromOrigin(0, 1, 0);
    EXPECT_EQ(vector, fromOrigin(0, 0, 1));
}

TEST(Vector3DTest, MultiplicationByNumberFromBothSides) {
    const Vector3D vector = fromOrigin(1, -2, 3);
    EXPECT_EQ(vector * 2.0, fromOrigin(2, -4, 6));
    EXPECT_EQ(2.0 * vector, fromOrigin(2, -4, 6));
}

TEST(Vector3DTest, CompoundMultiplicationByNumber) {
    Vector3D vector(Point3D(1, 1, 1), Point3D(2, 2, 2));
    vector *= 3.0;
    EXPECT_EQ(vector, Vector3D(Point3D(1, 1, 1), Point3D(4, 4, 4)));
}

TEST(Vector3DTest, DivisionByNumber) {
    const Vector3D vector = fromOrigin(2, 4, 6);
    EXPECT_EQ(vector / 2.0, fromOrigin(1, 2, 3));
    Vector3D copy = vector;
    copy /= 4.0;
    EXPECT_EQ(copy, fromOrigin(0.5, 1, 1.5));
}

TEST(Vector3DTest, DivisionByZeroThrows) {
    EXPECT_THROW(fromOrigin(1, 1, 1) / 0.0, std::domain_error);
}

TEST(Vector3DTest, CosineOfPerpendicularVectorsIsZero) {
    EXPECT_NEAR(fromOrigin(1, 0, 0).cosineWith(fromOrigin(0, 5, 0)), 0.0, 1e-12);
}

TEST(Vector3DTest, CosineOfSameAndOppositeDirections) {
    EXPECT_NEAR(fromOrigin(1, 2, 3).cosineWith(fromOrigin(2, 4, 6)), 1.0, 1e-12);
    EXPECT_NEAR(fromOrigin(1, 2, 3).cosineWith(fromOrigin(-1, -2, -3)), -1.0, 1e-12);
}

TEST(Vector3DTest, CaretOperatorReturnsCosine) {
    const Vector3D left = fromOrigin(1, 2, 3);
    const Vector3D right = fromOrigin(1, 2, 0);
    EXPECT_NEAR(left ^ right, 5.0 / std::sqrt(70.0), 1e-12);
}

TEST(Vector3DTest, CosineWithZeroVectorThrows) {
    EXPECT_THROW(fromOrigin(1, 2, 3) ^ Vector3D(), std::domain_error);
    EXPECT_THROW(Vector3D() ^ fromOrigin(1, 2, 3), std::domain_error);
}

TEST(Vector3DTest, ComparisonByLength) {
    const Vector3D shorter = fromOrigin(1, 0, 0);
    const Vector3D longer = fromOrigin(0, 0, 2);
    EXPECT_TRUE(shorter < longer);
    EXPECT_TRUE(shorter <= longer);
    EXPECT_TRUE(longer > shorter);
    EXPECT_TRUE(longer >= shorter);
    EXPECT_FALSE(longer < shorter);
    EXPECT_FALSE(shorter > longer);
}

TEST(Vector3DTest, VectorsOfEqualLengthAreNotOrderedStrictly) {
    const Vector3D first = fromOrigin(1, 0, 0);
    const Vector3D second = fromOrigin(0, 1, 0);
    EXPECT_TRUE(first.hasSameLengthAs(second));
    EXPECT_TRUE(first <= second);
    EXPECT_TRUE(first >= second);
    EXPECT_FALSE(first < second);
    EXPECT_FALSE(first > second);
    EXPECT_TRUE(first != second);
}

TEST(Vector3DTest, EqualityComparesBothEnds) {
    const Vector3D first(Point3D(0, 0, 0), Point3D(1, 1, 1));
    const Vector3D shifted(Point3D(1, 1, 1), Point3D(2, 2, 2));
    EXPECT_TRUE(first == first);
    EXPECT_TRUE(first != shifted);
    EXPECT_FALSE(first == shifted);
}

TEST(Vector3DTest, CopyAndAssignmentAreIndependent) {
    Vector3D original = fromOrigin(1, 2, 3);
    Vector3D copy(original);
    Vector3D assigned;
    assigned = original;
    original *= 2.0;
    EXPECT_EQ(copy, fromOrigin(1, 2, 3));
    EXPECT_EQ(assigned, fromOrigin(1, 2, 3));
}

TEST(Vector3DTest, StreamOutputFormat) {
    std::ostringstream out;
    out << Vector3D(Point3D(1, 2, 3), Point3D(4, 5, 6));
    EXPECT_EQ(out.str(), "[(1, 2, 3), (4, 5, 6)]");
}

TEST(Vector3DTest, StreamInputParsesFormat) {
    std::istringstream in("[(1, 2, 3), (4, 5, 6)]");
    Vector3D vector;
    in >> vector;
    EXPECT_TRUE(static_cast<bool>(in));
    EXPECT_EQ(vector, Vector3D(Point3D(1, 2, 3), Point3D(4, 5, 6)));
}

TEST(Vector3DTest, StreamInputRejectsBadFormatAndKeepsValue) {
    std::istringstream in("[(1, 2, 3) (4, 5, 6)]");
    Vector3D vector = fromOrigin(9, 9, 9);
    in >> vector;
    EXPECT_TRUE(in.fail());
    EXPECT_EQ(vector, fromOrigin(9, 9, 9));
}

TEST(Vector3DTest, OutputThenInputRoundTrip) {
    const Vector3D original(Point3D(-1.5, 2, 0.25), Point3D(3, -4, 5));
    std::stringstream stream;
    stream << original;
    Vector3D restored;
    stream >> restored;
    EXPECT_EQ(restored, original);
}
