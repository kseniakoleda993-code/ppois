/**
 * @file Menu.h
 * @author Коледа Ксения
 * @brief Универсальное консольное меню, не зависящее от предметных классов.
 */
#ifndef LAB1_CLI_MENU_H
#define LAB1_CLI_MENU_H

#include <functional>
#include <ios>
#include <iosfwd>
#include <istream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace lab1 {

/**
 * @brief Нумерованное меню с пунктами-действиями.
 * @details Ввод и вывод передаются через потоки, поэтому меню можно тестировать без консоли.
 * Исключения, брошенные действием, перехватываются и выводятся пользователю.
 */
class Menu {
public:
    /// Действие, выполняемое при выборе пункта.
    using Action = std::function<void()>;

    /**
     * @brief Создаёт меню.
     * @param title заголовок
     * @param in поток ввода
     * @param out поток вывода
     * @param exitCaption подпись пункта выхода (он всегда имеет номер 0)
     */
    Menu(std::string title, std::istream& in, std::ostream& out, std::string exitCaption = "Назад");

    /// Добавляет пункт в конец меню.
    void addItem(std::string caption, Action action);

    /// Запускает цикл меню; завершается при вводе 0 или конце потока ввода.
    void run();

private:
    struct Item {
        std::string caption;
        Action action;
    };

    void printItems() const;
    void executeChoice(const std::string& line);

    std::string title_;
    std::string exitCaption_;
    std::istream& in_;
    std::ostream& out_;
    std::vector<Item> items_;
};

/**
 * @brief Запрашивает у пользователя значение произвольного типа.
 * @tparam Value тип, для которого определён operator>>
 * @param in поток ввода
 * @param out поток вывода
 * @param message приглашение к вводу
 * @return введённое значение
 * @throws std::runtime_error если введённый текст не удалось разобрать
 */
template <typename Value>
Value promptValue(std::istream& in, std::ostream& out, const std::string& message) {
    out << message;
    Value value{};
    const bool success = static_cast<bool>(in >> value);
    in.clear();
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!success) {
        throw std::runtime_error("Некорректный ввод");
    }
    return value;
}

}  // namespace lab1

#endif  // LAB1_CLI_MENU_H
