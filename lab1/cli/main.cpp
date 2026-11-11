
/**
 * @file main.cpp
 * @author Коледа Ксения
 * @brief Точка входа: главное меню лабораторной работы №1.
 */
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "BigIntegerMenu.h"
#include "Menu.h"
#include "VectorMenu.h"

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    lab1::Menu mainMenu("Лабораторная работа №1", std::cin, std::cout, "Выход");
    lab1::VectorMenu vectorMenu(std::cin, std::cout);
    lab1::BigIntegerMenu bigIntegerMenu(std::cin, std::cout);
    mainMenu.addItem("Вектор в трёхмерном пространстве", [&vectorMenu] { vectorMenu.run(); });
    mainMenu.addItem("Длинное целое со знаком", [&bigIntegerMenu] { bigIntegerMenu.run(); });
    mainMenu.run();
    return 0;
}
