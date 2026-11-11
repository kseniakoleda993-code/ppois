/**
 * @file BigIntegerMenu.cpp
 * @author Коледа Ксения
 * @brief Реализация консольного меню для BigInteger.
 */
#include "BigIntegerMenu.h"

#include <istream>
#include <ostream>

namespace lab1 {

BigIntegerMenu::BigIntegerMenu(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

void BigIntegerMenu::buildMenu(Menu& menu) {
    menu.addItem("Ввести число A", [this] { readFirst(); });
    menu.addItem("Ввести число B", [this] { readSecond(); });
    menu.addItem("Показать числа", [this] { showNumbers(); });
    buildOperationItems(menu);
}

void BigIntegerMenu::buildOperationItems(Menu& menu) {
    menu.addItem("A + B", [this] { printSum(); });
    menu.addItem("A - B", [this] { printDifference(); });
    menu.addItem("A * B", [this] { printProduct(); });
    menu.addItem("A / B", [this] { printQuotient(); });
    menu.addItem("++A / A++", [this] { incrementFirst(); });
    menu.addItem("--A / A--", [this] { decrementFirst(); });
    menu.addItem("Сравнить A и B", [this] { printComparisons(); });
    menu.addItem("Операции A с обычным целым", [this] { printWithInteger(); });
    menu.addItem("Привести A к long long", [this] { printAsBuiltInInteger(); });
    menu.addItem("Вычислить n! (демонстрация больших чисел)", [this] { printFactorial(); });
}

void BigIntegerMenu::run() {
    Menu menu("Длинное целое со знаком", in_, out_);
    buildMenu(menu);
    menu.run();
}

void BigIntegerMenu::readFirst() {
    first_ = promptValue<BigInteger>(in_, out_, "Введите целое число: ");
}

void BigIntegerMenu::readSecond() {
    second_ = promptValue<BigInteger>(in_, out_, "Введите целое число: ");
}

void BigIntegerMenu::showNumbers() {
    out_ << "A = " << first_ << "\nB = " << second_ << "\n";
}

void BigIntegerMenu::printSum() { out_ << "A + B = " << first_ + second_ << "\n"; }

void BigIntegerMenu::printDifference() { out_ << "A - B = " << first_ - second_ << "\n"; }

void BigIntegerMenu::printProduct() { out_ << "A * B = " << first_ * second_ << "\n"; }

void BigIntegerMenu::printQuotient() { out_ << "A / B = " << first_ / second_ << "\n"; }

void BigIntegerMenu::incrementFirst() {
    const BigInteger previous = first_++;
    out_ << "A++ вернул " << previous << ", теперь A = " << first_ << "\n";
    out_ << "++A вернул " << ++first_ << "\n";
}

void BigIntegerMenu::decrementFirst() {
    const BigInteger previous = first_--;
    out_ << "A-- вернул " << previous << ", теперь A = " << first_ << "\n";
    out_ << "--A вернул " << --first_ << "\n";
}

void BigIntegerMenu::printComparisons() {
    out_ << "A == B: " << (first_ == second_) << "\nA != B: " << (first_ != second_)
         << "\nA > B: " << (first_ > second_) << "\nA >= B: " << (first_ >= second_)
         << "\nA < B: " << (first_ < second_) << "\nA <= B: " << (first_ <= second_) << "\n";
}

void BigIntegerMenu::printWithInteger() {
    const long long number = promptValue<long long>(in_, out_, "Введите обычное целое: ");
    out_ << "A + " << number << " = " << first_ + number << "\n";
    out_ << "A - " << number << " = " << first_ - number << "\n";
    out_ << "A * " << number << " = " << first_ * number << "\n";
    out_ << "A / " << number << " = " << first_ / number << "\n";
    out_ << "A > " << number << ": " << (first_ > number) << "\n";
}

void BigIntegerMenu::printAsBuiltInInteger() {
    out_ << "A как long long: " << static_cast<long long>(first_) << "\n";
}

void BigIntegerMenu::printFactorial() {
    const long long limit = promptValue<long long>(in_, out_, "Введите n: ");
    BigInteger factorial(1);
    for (long long factor = 2; factor <= limit; ++factor) {
        factorial *= factor;
    }
    out_ << limit << "! = " << factorial << "\n";
}

}  // namespace lab1
