#ifndef PROTEI_COV_RECORDER_HPP
#define PROTEI_COV_RECORDER_HPP
#include "commonStructures.hpp"

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

/**
 * @file recorder.hpp
 * @brief Содержит объявления интерфейса IRecorder и его конкретной реализации FileRecorder.
 */

/**
 * @class IRecorder
 * @brief Интерфейс для записи данных о вызове (CDR - Call Detail Record).
 */
class IRecorder {
public:
    /**
     * @brief Чисто виртуальная функция для создания записи о вызове.
     * @param cdr Структура CDR с данными о вызове.
     */
    virtual void makeRecord(const CDR& cdr) = 0;

    virtual void setLogger(std::shared_ptr<spdlog::logger> logger) = 0;

    /// @brief Виртуальный деструктор интерфейса.
    virtual ~IRecorder() {}

    std::mutex writeMutex_;
    std::shared_ptr<spdlog::logger> logger_;
};

/**
 * @class FileRecoreder
 * @brief Класс-наследник интерфейса IRecorder, реализующий запись данных о вызове в файл.
 */
class FileRecorder : public IRecorder {
public:
    FileRecorder(const std::filesystem::path &path);
    /**
     * @brief Реализация функции для создания записи о вызове в файле.
     * @param cdr Структура CDR с данными о вызове.
     */
    void makeRecord(const CDR& cdr) override;


    void setLogger(std::shared_ptr<spdlog::logger> logger);


private:
    std::ofstream file;
};


#endif