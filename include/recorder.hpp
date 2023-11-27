#include "commonStructures.hpp"

/**
 * @file recorder.hpp
 * @brief Содержит объявления интерфейса IRecoreder и его конкретной реализации FileRecoreder.
 */

/**
 * @class IRecoreder
 * @brief Интерфейс для записи данных о вызове (CDR - Call Detail Record).
 */
class IRecoreder {
public:
    /**
     * @brief Чисто виртуальная функция для создания записи о вызове.
     * @param cdr Структура CDR с данными о вызове.
     */
    virtual void makeRecord(CDR& cdr) = 0;

    /// @brief Виртуальный деструктор интерфейса.
    virtual ~IRecoreder() {}
};

/**
 * @class FileRecoreder
 * @brief Класс-наследник интерфейса IRecoreder, реализующий запись данных о вызове в файл.
 */


class FileRecoreder : public IRecoreder {
public:
    /**
     * @brief Реализация функции для создания записи о вызове в файле.
     * @param cdr Структура CDR с данными о вызове.
     */
    void MakeRecord(CDR& cdr);
};