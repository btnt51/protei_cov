
#ifndef PROTEI_COV_CONFIG_HPP
#define PROTEI_COV_CONFIG_HPP
#include <filesystem>
#include <thread>
#include <chrono>

#include "interfaces.hpp"

namespace utility {

/**
 * @brief Класс Config представляет собой реализацию интерфейса IConfig.
 *
 * @copydoc utility::IConfig
 */
class Config : public utility::IConfig {
public:
    /**
     * @copydoc IConfig::IConfig
     */
    explicit Config(const std::filesystem::path& path, std::shared_ptr<spdlog::logger> logger);

    /**
     * @copydoc IConfig::getMinMax
     */
    std::pair<int, int> getMinMax() override;

    /**
     * @copydoc IConfig::getAmountOfOperators
     */
    int getAmountOfOperators() override;

    /**
     * @copydoc IConfig::getSizeOfQueue
     */
    int getSizeOfQueue() override;

    /**
     * @copydoc IConfig::getPath
     */
    std::filesystem::path getPath() override;

    /**
     * @copydoc IConfig::updateConfig
     */
    void updateConfig() override;

    /**
     * @copydoc IConfig::updateViaRequest
     */
    void updateWithRequest() override;

    /**
     * @copydoc IConfig::isUpdated
     */
    bool isUpdated() override;

    /**
     * @copydoc IConfig::notify
     */
    void notify() override;

    /**
     * @copydoc IConfig::setManager
     */
    void setManager(std::shared_ptr<IManager> manager) override;

    /**
     * @copydoc IConfig::setLogger
     */
    void setLogger(std::shared_ptr<spdlog::logger> logger) override;

private:
    /**
     * @brief Приводит путь к нормальному виду для корректной обработки.
     * @param pathToFile Путь к файлу.
     * @return Нормализованный путь.
     */
    static std::filesystem::path makeNormalPath(const std::filesystem::path& pathToFile);

    std::shared_ptr<JsonParser> parser; ///< Парсер JSON-файлов для обработки конфигурации.
    std::filesystem::path path_; ///< Путь к файлу конфигурации.
    std::map<std::string, int> data_; ///< Данные конфигурации.
    std::shared_ptr<IManager> manager; ///< Указатель на объект менеджера для обработки изменений конфигурации.
    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер
};

/**
 * @brief Класс ThreadSafeConfig представляет собой потокобезопасную реализацию интерфейса IConfig.
 *
 * @copydoc utility::IConfig
 */
class ThreadSafeConfig : public utility::IConfig {
public:
    /**
     * @copydoc IConfig::IConfig
     */
    explicit ThreadSafeConfig(const std::filesystem::path& path, std::shared_ptr<spdlog::logger> logger);

    /**
     * @copydoc IConfig::~IConfig
     */
    ~ThreadSafeConfig();

    /**
     * @copydoc IConfig::getMinMax
     */
    std::pair<int, int> getMinMax() override;

    /**
     * @copydoc IConfig::getAmountOfOperators
     */
    int getAmountOfOperators() override;

    /**
     * @copydoc IConfig::getSizeOfQueue
     */
    int getSizeOfQueue() override;

    /**
     * @copydoc IConfig::getPath
     */
    std::filesystem::path getPath() override;

    /**
     * @copydoc IConfig::updateConfig
     */
    void updateConfig() override;

    /**
     * @copydoc IConfig::updateViaRequest
     */
    void updateWithRequest() override;

    /**
     * @copydoc IConfig::isUpdated
     */
    bool isUpdated() override;

    /**
     * @copydoc IConfig::updateConfigThread
     */
    void updateConfigThread();

    /**
     * @copydoc IConfig::notify
     */
    void notify() override;

    /**
     * @copydoc IConfig::setManager
     */
    void setManager(std::shared_ptr<IManager> manager) override;

    /**
     * @copydoc IConfig::setLogger
     */
    void setLogger(std::shared_ptr<spdlog::logger> logger) override;

    /**
     * @brief Запускает мониторинг конфигурации в отдельном потоке.
     */
    void RunMonitoring();

    /**
     * @brief Проверяет, выполняется ли мониторинг конфигурации.
     * @return true, если мониторинг выполняется, в противном случае - false.
     */
    bool isMonitoring() const;

private:
    /**
     * @brief Приводит путь к нормальному виду для корректной обработки.
     * @param pathToFile Путь к файлу.
     * @return Нормализованный путь.
     */
    static std::filesystem::path makeNormalPath(const std::filesystem::path& pathToFile);

    std::shared_ptr<JsonParser> parser; ///< Парсер JSON-файлов для обработки конфигурации.
    std::filesystem::path path_; ///< Путь к файлу конфигурации.
    std::map<std::string, int> data_; ///< Данные конфигурации.
    std::thread updateThread; ///< Поток для асинхронного обновления конфигурации.
    std::mutex configMutex; ///< Мьютекс для защиты доступа к конфигурации.
    bool stopThread; ///< Флаг для остановки потока обновления конфигурации.
    bool updated; ///< Флаг, указывающий на обновление конфигурации.
    std::time_t lastWriteTime; ///< Время последнего изменения файла конфигурации.
    std::shared_ptr<IManager> manager; ///< Указатель на объект менеджера для обработки изменений конфигурации.
    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер
};

}
#endif