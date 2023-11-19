#ifndef PROTEI_COV_MANAGER_HPP
#define PROTEI_COV_MANAGER_HPP
#include <filesystem>
#include <future>

#include "commonStructures.hpp"
#include "jsonParser.hpp"
#include "threadpool.hpp"
/**
 * @class Manager
 * @brief Класс управляющей тредпулом (центром обратки вызовов)
 */
class Manager {
public:
    /**
     * @brief Конструктор класса Manager.
     * @param pathToConfig Путь к файлу конфигурации.
     */
    Manager(const std::string& pathToConfig);

    /**
     * @brief Запуск мониторинга файла конфигурации.
     */
    void startMonitoringConfigFile();

    /**
     * @brief Остановка мониторинга файла конфигурации.
     */
    void stopMonitoringConfigFile();

    /**
     * @brief Добавление задачи в тредпул.
     * @param number Номер вызова.
     * @return Пара, содержащая уникальный идентификатор вызова и будущий результат выполнения задачи.
     */
    std::pair<TP::CallID, std::future<Result>> addTask(std::string_view number);

private:
    /**
     * @brief Метод для мониторинга файла конфигурации. На данный момент заглушка
     */
    void monitorConfigFile();

    /**
     * @brief Метод для обновления тредпула. На данный момент заглушка
     */
    void updateThreadPool();

    const std::filesystem::path pathToConfigFile; ///< Путь к файлу конфигурации.

    std::shared_ptr<TP::ThreadPool> threadPool; ///< Указатель на объект тредпула.

    std::atomic<bool> stopMonitoring; ///< Флаг остановки мониторинга файла.

    std::thread monitorThread; ///< Поток для мониторинга файла конфигурации.

    /// @brief Верхняя граница.
    int RMin_;

    /// @brief Нижняя граница.
    int RMax_;

    utility::JsonParser parser; ///< Объект парсера JSON.
};

#endif // PROTEI_COV_MANAGER_HPP