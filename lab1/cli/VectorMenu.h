/**
 * @file VectorMenu.h
 * @author Коледа Ксения
 * @brief Консольное меню для работы с классом Vector3D.
 */
#ifndef LAB1_CLI_VECTORMENU_H
#define LAB1_CLI_VECTORMENU_H

#include <iosfwd>

#include "Menu.h"
#include "Vector3D.h"

namespace lab1 {

/**
 * @brief Интерактивная сессия с двумя векторами A и B.
 * @details Класс знает о Vector3D, но Vector3D ничего не знает о консоли.
 */
class VectorMenu {
public:
    /// @param in поток ввода
    /// @param out поток вывода
    VectorMenu(std::istream& in, std::ostream& out);

    /// Запускает меню.
    void run();

private:
    void buildMenu(Menu& menu);
    void readFirst();
    void readSecond();
    void showVectors();
    void printSum();
    void printDifference();
    void printCrossProduct();
    void printScaled();
    void printDivided();
    void printLengths();
    void printCosine();
    void printLengthComparison();
    void printEquality();

    std::istream& in_;
    std::ostream& out_;
    Vector3D first_;
    Vector3D second_;
};

}  // namespace lab1

#endif  // LAB1_CLI_VECTORMENU_H
