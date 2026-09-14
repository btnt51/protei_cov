#ifndef PROTEI_COV_MANAGER_HPP
#define PROTEI_COV_MANAGER_HPP
#include "interfaces.hpp"
#include <shared_mutex>
/**
 * @file manager.hpp
 * @brief Содержит объявление класса Manager, который реализует интерфейс IManager.
 */

/**
 * @class Manager
 * @brief Класс управляющей тредпулом (центром обратки вызовов)
 *
 * @copydoc IManager
 */
class Manager : public IManager, std::enable_shared_from_this<Manager> {
public:
    /**
     * @brief Конструктор класса Manager.
     * @param pathToConfig Путь к файлу конфигурации.
     *
     * @copydoc IManager::IManager
     */
    explicit Manager(std::shared_ptr<utility::IConfig> conf, std::shared_ptr<TP::IThreadPool>);

    /**
     * @brief Добавление задачи в тредпул.
     * @param number Номер вызова.
     * @return Пара, содержащая уникальный идентификатор вызова и будущий результат выполнения задачи.
     *
     * @copydoc IManager::addTask
     */
    std::pair<TP::CallID, std::future<Result>> addTask(std::string_view number) override;

    /**
     * @brief Запуск тредпула.
     *
     * @copydoc IManager::startThreadPool
     */
    void startThreadPool() override;

    /**
     * @brief Остановка тредпула.
     *
     * @copydoc IManager::stopThreadPool
     */
    void stopThreadPool() override;

    /**
     * @brief Установка новой конфигурации и обновления ее методом IManager::update
     * @param config Новый объект конфигурации.
     *
     * @copydoc IManager::setNewConfig
     */
    void setNewConfig(std::shared_ptr<utility::IConfig> config) override;

    /**
     * @brief Установка нового тредпула, с ожиданием выполнения всех
     * задач в работе в старом тредпуле, переносом задач из старого в новый
     * и запуск нового тредпула
     * @param pool Новый объект тредпула.
     *
     * @copydoc IManager::setNewThreadPool
     */
    void setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) override;

    /**
     * @brief Метод для обновления тредпула.
     * @copydoc IManager::update
     */
    void update() override;

    /**
     * @brief Устанавливает асинхронный логгер.
     * @param logger Указатель на объект логгера.
     * @copydoc IManager::setLogger
     */
    void setLogger(std::shared_ptr<spdlog::logger> logger) override;

    /**
     * @brief Обрабатывает запрос на обновление конфигурации
     * @return true, если конфигурация была обновлена, в противном случае - false.
     * @copydoc IManager::processRequestForUpdate
     */
    bool processRequestForUpdate() override;

private:
    std::shared_mutex updateMtx; ///< Мьютекс для обеспечения безопасного доступа к обновлению.

    int RMin_; ///< Верхняя граница.

    int RMax_;  ///< Нижняя граница.

    std::shared_ptr<utility::IConfig> config_; ///< Указатель на объект конфигурации.
    std::shared_ptr<TP::IThreadPool> threadPool_; ///< Указатель на объект тредпула.
    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер.
};


#endif // PROTEI_COV_MANAGER_HPP