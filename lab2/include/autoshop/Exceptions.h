#pragma once

#include <stdexcept>
#include <string>

namespace autoshop {

/// Базовое исключение предметной области «Автомастерская».
class AutoShopException : public std::runtime_error {
public:
    explicit AutoShopException(const std::string& message) : std::runtime_error(message) {}
};

/// Некорректные данные клиента.
class InvalidClientDataException : public AutoShopException {
public:
    explicit InvalidClientDataException(const std::string& details)
        : AutoShopException("Некорректные данные клиента: " + details) {}
};

/// Некорректные данные автомобиля.
class InvalidVehicleDataException : public AutoShopException {
public:
    explicit InvalidVehicleDataException(const std::string& details)
        : AutoShopException("Некорректные данные автомобиля: " + details) {}
};

/// Недопустимая операция с деньгами (разные валюты, переплата, неизвестный способ оплаты).
class InvalidMoneyOperationException : public AutoShopException {
public:
    explicit InvalidMoneyOperationException(const std::string& details)
        : AutoShopException("Недопустимая денежная операция: " + details) {}
};

/// Недостаточно средств (или баллов, или лимита).
class InsufficientFundsException : public AutoShopException {
public:
    explicit InsufficientFundsException(const std::string& details)
        : AutoShopException("Недостаточно средств: " + details) {}
};

/// Запчасти нет в нужном количестве на складе.
class OutOfStockException : public AutoShopException {
public:
    explicit OutOfStockException(const std::string& partName)
        : AutoShopException("Недостаточно на складе: " + partName) {}
};

/// Некорректные данные запчасти, поставщика или позиции заказа.
class InvalidPartDataException : public AutoShopException {
public:
    explicit InvalidPartDataException(const std::string& details)
        : AutoShopException("Некорректные данные запчасти: " + details) {}
};

/// Некорректные данные услуги.
class InvalidServiceDataException : public AutoShopException {
public:
    explicit InvalidServiceDataException(const std::string& details)
        : AutoShopException("Некорректные данные услуги: " + details) {}
};

/// Механик занят или не имеет нужной квалификации.
class MechanicNotAvailableException : public AutoShopException {
public:
    explicit MechanicNotAvailableException(const std::string& details)
        : AutoShopException("Механик недоступен: " + details) {}
};

/// Нет свободного рабочего поста (бокса).
class BayNotAvailableException : public AutoShopException {
public:
    explicit BayNotAvailableException(const std::string& details)
        : AutoShopException("Пост недоступен: " + details) {}
};

/// Конфликт записи на обслуживание или смены.
class AppointmentConflictException : public AutoShopException {
public:
    explicit AppointmentConflictException(const std::string& details)
        : AutoShopException("Конфликт расписания: " + details) {}
};

/// Операция недопустима в текущем состоянии заказа.
class InvalidOrderStateException : public AutoShopException {
public:
    explicit InvalidOrderStateException(const std::string& details)
        : AutoShopException("Недопустимое состояние заказа: " + details) {}
};

/// Некорректная дата или временной интервал.
class InvalidDateException : public AutoShopException {
public:
    explicit InvalidDateException(const std::string& details)
        : AutoShopException("Некорректная дата или время: " + details) {}
};

/// Промокод недействителен или исчерпан.
class PromoCodeException : public AutoShopException {
public:
    explicit PromoCodeException(const std::string& code)
        : AutoShopException("Промокод недоступен: " + code) {}
};

/// Гарантия не действует.
class WarrantyExpiredException : public AutoShopException {
public:
    explicit WarrantyExpiredException(const std::string& details)
        : AutoShopException("Гарантия не действует: " + details) {}
};

/// Превышен лимит скидки.
class DiscountLimitExceededException : public AutoShopException {
public:
    explicit DiscountLimitExceededException(const std::string& details)
        : AutoShopException("Превышен лимит скидки: " + details) {}
};

/// Запрошенный объект не найден.
class NotFoundException : public AutoShopException {
public:
    explicit NotFoundException(const std::string& what)
        : AutoShopException("Не найдено: " + what) {}
};

}  // namespace autoshop
