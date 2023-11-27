#pragma once
#include <chrono>
#include <thread>

/** @file commonStructures.hpp
 *  @brief Содержит общие структуры и типы данных,
 *  используемые в рамках проекта.
 */

/**
 * @namespace TP
 * @brief Пространство имен TP, содержащее объявление типа CallID,
 * а также объявление и определение классов Task, Operator, ThreadPool
 */
namespace TP {
using CallID = long long;
}

/**
 * @enum CallStatus
 * @brief Перечисление, представляющее возможные статусы вызова.
 */
enum class CallStatus {
    awaiting,   ///< Ожидание обработки вызова.
    completed,  ///< Вызов успешно завершен.
    rejected    ///< Вызов отклонен.
};

/**
 * @struct CDR
 * @brief Структура CDR (Call Detail Record), представляющая запись о вызове.
 */
struct CDR {
    std::time_t startTime;                 ///< Время начала вызова.
    std::time_t endTime;                   ///< Время окончания вызова.
    TP::CallID callID;                    ///< Уникальный идентификатор вызова.
    CallStatus status;                    ///< Статус вызова (ожидание, завершено, отклонено).
    std::thread::id operatorID;           ///< Идентификатор потока оператора, обработавшего вызов.
    std::chrono::duration<int> callDuration; ///< Продолжительность вызова в секундах.
    std::string_view number;                     ///< Номер вызова.
};

/**
 * @struct Result
 * @brief Структура Result представляет результат обработки вызова.
 */
struct Result {
    CallStatus status; ///< Статус вызова.
    std::chrono::duration<int> callDuration; ///< Продолжительность вызова в секундах.
    TP::CallID callID; ///< Уникальный идентификатор вызова.
};