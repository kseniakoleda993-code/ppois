/**
 * @file StreamParsing.h
 * @author Коледа Ксения
 * @brief Вспомогательная функция разбора текстовых потоков.
 */
#ifndef LAB1_STREAMPARSING_H
#define LAB1_STREAMPARSING_H

#include <iosfwd>

namespace lab1 {

/**
 * @brief Пропускает пробелы и считывает ожидаемый символ.
 * @param in входной поток
 * @param expected символ, который должен идти следующим
 * @return true, если символ найден и считан; иначе у потока выставляется failbit
 */
bool expectSymbol(std::istream& in, char expected);

}  // namespace lab1

#endif  // LAB1_STREAMPARSING_H
