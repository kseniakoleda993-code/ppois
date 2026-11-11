/**
 * @file StreamParsing.cpp
 * @author Коледа Ксения
 * @brief Реализация вспомогательных функций разбора потоков.
 */
#include "StreamParsing.h"

#include <istream>

namespace lab1 {

bool expectSymbol(std::istream& in, char expected) {
    in >> std::ws;
    if (in.peek() != static_cast<unsigned char>(expected)) {
        in.setstate(std::ios::failbit);
        return false;
    }
    in.get();
    return true;
}

}  // namespace lab1
