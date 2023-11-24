#ifndef PROTEI_COV_MANAGER_HPP
#define PROTEI_COV_MANAGER_HPP
#include "interfaces.hpp"
#include <shared_mutex>

/**
 * @class Manager
 * @brief Класс управляющей тредпулом (центром обратки вызовов)
 */
class Manager : public IManager, std::enable_shared_from_this<Manager> {
public:
    /**
     * @brief Конструктор класса Manager.
     * @param pathToConfig Путь к файлу конфигурации.
     */
    explicit Manager(std::shared_ptr<utility::IConfig> conf, std::shared_ptr<TP::IThreadPool>);


    /**
     * @brief Добавление задачи в тредпул.
     * @param number Номер вызова.
     * @return Пара, содержащая уникальный идентификатор вызова и будущий результат выполнения задачи.
     */
    std::pair<TP::CallID, std::future<Result>> addTask(std::string_view number) override;

    void startThreadPool() override;

    void stopThreadPool() override;

    void setNewConfig(std::shared_ptr<utility::IConfig> config) override;

    void setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) override;

    /**
     * @brief Метод для обновления тредпула. На данный момент заглушка
     */
    void update() override;

private:

    std::shared_mutex updateMtx;

    /// @brief Верхняя граница.
    int RMin_;

    /// @brief Нижняя граница.
    int RMax_;

    std::shared_ptr<utility::IConfig> config_;
    std::shared_ptr<TP::IThreadPool> threadPool_; ///< Указатель на объект тредпула.
};

#endif // PROTEI_COV_MANAGER_HPP