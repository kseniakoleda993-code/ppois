/**
 * @file BigIntegerMenu.h
 * @author Коледа Ксения
 * @brief Консольное меню для работы с классом BigInteger.
 */
#ifndef LAB1_CLI_BIGINTEGERMENU_H
#define LAB1_CLI_BIGINTEGERMENU_H

#include <iosfwd>

#include "BigInteger.h"
#include "Menu.h"

namespace lab1 {

/**
 * @brief Интерактивная сессия с двумя длинными числами A и B.
 * @details Класс знает о BigInteger, но BigInteger ничего не знает о консоли.
 */
class BigIntegerMenu {
public:
    /// @param in поток ввода
    /// @param out поток вывода
    BigIntegerMenu(std::istream& in, std::ostream& out);

    /// Запускает меню.
    void run();

private:
    void buildMenu(Menu& menu);
    void buildOperationItems(Menu& menu);
    void readFirst();
    void readSecond();
    void showNumbers();
    void printSum();
    void printDifference();
    void printProduct();
    void printQuotient();
    void incrementFirst();
    void decrementFirst();
    void printComparisons();
    void printWithInteger();
    void printAsBuiltInInteger();
    void printFactorial();

    std::istream& in_;
    std::ostream& out_;
    BigInteger first_;
    BigInteger second_;
};

}  // namespace lab1

#endif  // LAB1_CLI_BIGINTEGERMENU_H
