/**
 * @file Menu.cpp
 * @author Коледа Ксения
 * @brief Реализация универсального консольного меню.
 */
#include "Menu.h"

#include <istream>
#include <ostream>
#include <utility>

namespace lab1 {

namespace {
const char* const kExitChoice = "0";
}  // namespace

Menu::Menu(std::string title, std::istream& in, std::ostream& out, std::string exitCaption)
    : title_(std::move(title)), exitCaption_(std::move(exitCaption)), in_(in), out_(out) {}

void Menu::addItem(std::string caption, Action action) {
    items_.push_back(Item{std::move(caption), std::move(action)});
}

void Menu::printItems() const {
    out_ << "\n=== " << title_ << " ===\n";
    for (std::size_t index = 0; index < items_.size(); ++index) {
        out_ << index + 1 << ". " << items_[index].caption << "\n";
    }
    out_ << kExitChoice << ". " << exitCaption_ << "\n> ";
}

void Menu::executeChoice(const std::string& line) {
    std::size_t number = 0;
    try {
        number = std::stoul(line);
    } catch (const std::logic_error&) {
        number = 0;
    }
    if (number == 0 || number > items_.size()) {
        out_ << "Неизвестный пункт меню\n";
        return;
    }
    try {
        items_[number - 1].action();
    } catch (const std::exception& error) {
        out_ << "Ошибка: " << error.what() << "\n";
    }
}

void Menu::run() {
    std::string line;
    while (true) {
        printItems();
        if (!std::getline(in_, line) || line == kExitChoice) {
            return;
        }
        executeChoice(line);
    }
}

}  // namespace lab1
