
#ifndef PROTEI_COV_INTERFACES_HPP
#define PROTEI_COV_INTERFACES_HPP
#include <filesystem>
#include <future>
#include <memory>
#include <queue>

#include "commonStructures.hpp"
#include "jsonParser.hpp"

class IManager;
namespace utility {
class IConfig {
public:
    IConfig([[maybe_unused]] const std::filesystem::path& path) { }
    virtual ~IConfig() { }
    virtual std::pair<int, int> getMinMax() = 0;
    virtual int getAmountOfOperators() = 0;
    virtual int getSizeOfQueue() = 0;
    virtual std::filesystem::path getPath() = 0;
    virtual void updateConfig() = 0;
    virtual bool isUpdated() = 0;
    virtual void notify() = 0;
    virtual void setManager(std::shared_ptr<IManager> manager) = 0;
};
}


namespace TP {
class Task;
class IThreadPool {
public:
    IThreadPool([[maybe_unused]] unsigned amountOfThreads) {}

    virtual ~IThreadPool() {}

    virtual std::pair<CallID, std::future<Result>> add_task(const Task& task) = 0;

    virtual void stop() = 0;

    /// @brief запуск пула
    virtual void start() = 0;

    virtual void transferTaskQueue(const std::shared_ptr<IThreadPool>& oldThreadPool) = 0;

    std::mutex task_queue_mutex;
    std::queue<std::pair<std::shared_ptr<Task>, CallID>> task_queue;
};

}

class IManager {
public:
    /**
     * @brief Конструктор класса Manager.
     * @param pathToConfig Путь к файлу конфигурации.
     */
    IManager([[maybe_unused]] std::shared_ptr<utility::IConfig> conf, [[maybe_unused]] std::shared_ptr<TP::IThreadPool>) {}


    /**
     * @brief Добавление задачи в тредпул.
     * @param number Номер вызова.
     * @return Пара, содержащая уникальный идентификатор вызова и будущий результат выполнения задачи.
     */
    virtual std::pair<TP::CallID, std::future<Result>> addTask(std::string_view number) = 0;

    virtual void startThreadPool() = 0;

    virtual void stopThreadPool() = 0;

    virtual void setNewConfig(std::shared_ptr<utility::IConfig> config) = 0;

    virtual void setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) = 0;

    virtual void update() = 0;

};
#endif // PROTEI_COV_INTERFACES_HPP
