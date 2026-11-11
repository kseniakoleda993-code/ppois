/**
 * @file VectorMenu.cpp
 * @author Коледа Ксения
 * @brief Реализация консольного меню для Vector3D.
 */
#include "VectorMenu.h"

#include <istream>
#include <ostream>

namespace lab1 {

namespace {
const char* const kVectorPrompt = "Введите вектор в формате [(x, y, z), (x, y, z)]: ";
}  // namespace

VectorMenu::VectorMenu(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

void VectorMenu::buildMenu(Menu& menu) {
    menu.addItem("Ввести вектор A", [this] { readFirst(); });
    menu.addItem("Ввести вектор B", [this] { readSecond(); });
    menu.addItem("Показать векторы", [this] { showVectors(); });
    menu.addItem("A + B", [this] { printSum(); });
    menu.addItem("A - B", [this] { printDifference(); });
    menu.addItem("A * B (векторное произведение)", [this] { printCrossProduct(); });
    menu.addItem("A * число", [this] { printScaled(); });
    menu.addItem("A / число", [this] { printDivided(); });
    menu.addItem("Длины векторов", [this] { printLengths(); });
    menu.addItem("Косинус угла между A и B", [this] { printCosine(); });
    menu.addItem("Сравнить A и B по длине", [this] { printLengthComparison(); });
    menu.addItem("Проверить равенство A и B", [this] { printEquality(); });
}

void VectorMenu::run() {
    Menu menu("Вектор в трёхмерном пространстве", in_, out_);
    buildMenu(menu);
    menu.run();
}

void VectorMenu::readFirst() { first_ = promptValue<Vector3D>(in_, out_, kVectorPrompt); }

void VectorMenu::readSecond() { second_ = promptValue<Vector3D>(in_, out_, kVectorPrompt); }

void VectorMenu::showVectors() {
    out_ << "A = " << first_ << "\nB = " << second_ << "\n";
}

void VectorMenu::printSum() { out_ << "A + B = " << first_ + second_ << "\n"; }

void VectorMenu::printDifference() { out_ << "A - B = " << first_ - second_ << "\n"; }

void VectorMenu::printCrossProduct() { out_ << "A * B = " << first_ * second_ << "\n"; }

void VectorMenu::printScaled() {
    const double factor = promptValue<double>(in_, out_, "Введите число: ");
    out_ << "A * " << factor << " = " << first_ * factor << "\n";
}

void VectorMenu::printDivided() {
    const double divisor = promptValue<double>(in_, out_, "Введите число: ");
    out_ << "A / " << divisor << " = " << first_ / divisor << "\n";
}

void VectorMenu::printLengths() {
    out_ << "|A| = " << first_.length() << "\n|B| = " << second_.length() << "\n";
}

void VectorMenu::printCosine() { out_ << "cos(A, B) = " << (first_ ^ second_) << "\n"; }

void VectorMenu::printLengthComparison() {
    out_ << "A > B: " << (first_ > second_) << "\nA >= B: " << (first_ >= second_)
         << "\nA < B: " << (first_ < second_) << "\nA <= B: " << (first_ <= second_) << "\n";
}

void VectorMenu::printEquality() {
    out_ << "A == B: " << (first_ == second_) << "\nA != B: " << (first_ != second_) << "\n";
}

}  // namespace lab1
