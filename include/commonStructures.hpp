#ifndef PROTEI_COV_COMMONNSTRUCTURE_HPP
#define PROTEI_COV_COMMONNSTRUCTURE_HPP
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <string>

/** @file commonStructures.hpp
 *  @brief Содержит общие структуры и типы данных, а также функции
 *  необходимые для работы с этими структурами и типами данных.
 */

/**
 * @namespace TP
 * @brief Пространство имен TP, содержащее объявление типа CallID,
 * а также объявление и определение классов Task, Operator, ThreadPool
 */
namespace TP {
using CallID = unsigned long long;
}

/**
 * @enum CallStatus
 * @brief Перечисление, представляющее возможные статусы вызова.
 */

enum class CallStatus : int {
    Awaiting,   ///< Ожидание обработки вызова.
    Completed,  ///< Вызов успешно завершен.
    Duplication, ///< Повторный вызов.
    Overloaded, ///< Очередь перегружена.
    Rejected,   ///< Вызов отклонен.
    Timeout ///< Вызов не был обслужен.
};

namespace utility {
/**
 * @brief Преобразует перечисление CallStatus в строковое представление.
 * @param status Статус вызова (CallStatus).
 * @return Строковое представление статуса вызова.
 */
std::string to_string(const CallStatus& status);

/**
 * @brief Подготавливает строковое представление времени из временной точки системных часов,
 * если timeP – пустое представление (конструктор по-умолчанию), вернется пустая строка.
 * @param timeP Временная точка системных часов (std::chrono::system_clock::time_point).
 * @return Строковое представление времени.
 */
std::string prepareTime(const std::chrono::system_clock::time_point& timeP);
}

/**
 * @struct CDR
 * @brief Структура CDR (Call Detail Record), представляющая запись о вызове.
 */
struct CDR {
    std::chrono::system_clock::time_point startTime;                 ///< Время начала вызова.
    std::chrono::system_clock::time_point endTime;                   ///< Время окончания вызова.
    TP::CallID callID;                    ///< Уникальный идентификатор вызова.
    CallStatus status;                    ///< Статус вызова (ожидание, завершено, отклонено, завис, повторный вызов,
                                          ///< Очередь перегружена).
    std::chrono::system_clock::time_point operatorCallTime;      ///< Время, когда оператор начал обрабатывать вызов.
    std::size_t operatorID;           ///< Идентификатор потока оператора, обработавшего вызов.
    std::chrono::duration<int> callDuration; ///< Продолжительность вызова в секундах.
    std::string_view number;                     ///< Номер вызова.
};
std::ofstream& operator<<(std::ofstream& out, const CDR& cdr);

/**
 * @struct Result
 * @brief Структура Result представляет результат обработки вызова.
 */
struct Result {
    CallStatus status; ///< Статус вызова.
    std::chrono::duration<int> callDuration; ///< Продолжительность вызова в секундах.
    TP::CallID callID; ///< Уникальный идентификатор вызова.
};

#endif