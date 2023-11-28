
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
/**
 * @brief Абстрактный интерфейс для работы с конфигурацией.
 */
class IConfig {
public:
    /**
     * @brief Конструктор класса IConfig.
     * @param path Путь к файлу конфигурации.
     */
    IConfig([[maybe_unused]] const std::filesystem::path& path) {}

    /**
     * @brief Виртуальный деструктор класса IConfig.
     */
    virtual ~IConfig() {}

    /**
     * @brief Возвращает пару значений - минимальное и максимальное количество операторов.
     * @return Пара значений: (минимум, максимум).
     */
    virtual std::pair<int, int> getMinMax() = 0;

    /**
     * @brief Возвращает количество операторов из конфигурации.
     * @return Количество операторов.
     */
    virtual int getAmountOfOperators() = 0;

    /**
     * @brief Возвращает размер очереди из конфигурации.
     * @return Размер очереди.
     */
    virtual int getSizeOfQueue() = 0;

    /**
     * @brief Возвращает путь к файлу конфигурации.
     * @return Путь к файлу конфигурации.
     */
    virtual std::filesystem::path getPath() = 0;

    /**
     * @brief Обновляет данные конфигурации.
     */
    virtual void updateConfig() = 0;

    /**
     * @brief Проверяет, была ли конфигурация обновлена.
     * @return true, если конфигурация была обновлена, в противном случае - false.
     */
    virtual bool isUpdated() = 0;

    /**
     * @brief Уведомляет менеджера о необходимости обновления конфигурации.
     */
    virtual void notify() = 0;

    /**
     * @brief Устанавливает менеджера для обработки конфигурационных изменений.
     * @param manager Указатель на объект менеджера.
     */
    virtual void setManager(std::shared_ptr<IManager> manager) = 0;
};

}


namespace TP {
class IThreadPool;
class ITask {
public:
    /**
     * @brief конструктор
     * @param RMin нижняя граница времени
     * @param RMax верхняя граница времени
     * @param number номер звонящего
     * @param time время создания задачи
     */
    ITask([[maybe_unused]] int RMin, [[maybe_unused]] int RMax, [[maybe_unused]] std::string_view number,
          [[maybe_unused]] std::time_t& time) {}


    /// @brief Обработка вызова.
    virtual Result doTask() = 0;

    /**
     * @brief Установка ID вызова.
     * @param id ID вызова.
     */
    virtual void setCallID(CallID& id) = 0;

    /**
     * @brief Установка ID потока.
     * @param id ID потока.
     */
    virtual void setThreadID(std::thread::id& id) = 0;

    /**
     * @brief Функция добавления promise в задачу
     * @param promise промис для уведомления о выполнения задачи
     */
    virtual void addPromise(std::shared_ptr<std::promise<Result>> promise) = 0;

    virtual void sendCDR() = 0;

    virtual std::string_view getNumber() = 0;

    /// @brief Дружественный класс.
    friend class IThreadPool;

    std::shared_ptr<std::promise<Result>> promise_;

    /// @brief Пул потоков.
    std::shared_ptr<IThreadPool> pool_;
};

class IQueue {
public:
    IQueue([[maybe_unused]] int size) {}

    virtual std::pair<std::shared_ptr<ITask>, CallID> back() = 0;

    virtual std::pair<std::shared_ptr<ITask>, CallID> front() = 0;

    [[nodiscard]] virtual bool empty() const = 0;

    virtual bool push(std::pair<std::shared_ptr<ITask>, CallID>&& taskPair) = 0;

    virtual void pop() = 0;

    virtual void update(int size) = 0;
};

/**
 * @brief Абстрактный интерфейс для управления пулом потоков.
 */
class IThreadPool {
public:
    /**
     * @brief Конструктор класса IThreadPool.
     * @param amountOfThreads Количество потоков в пуле.
     */
    IThreadPool([[maybe_unused]] unsigned amountOfThreads, [[maybe_unused]] unsigned sizeOfQueue) { }

    /**
     * @brief Виртуальный деструктор класса IThreadPool.
     */
    virtual ~IThreadPool() { }

    /**
     * @brief Добавляет задачу в пул потоков.
     * @param task Задача для выполнения в потоке.
     * @return Пара, содержащая уникальный идентификатор вызова и будущий результат выполнения задачи.
     */
    virtual std::pair<CallID, std::future<Result>> add_task(std::shared_ptr<ITask> task) = 0;

    /**
     * @brief Останавливает выполнение задач в пуле потоков.
     */
    virtual void stop() = 0;

    /**
     * @brief Запускает выполнение задач в пуле потоков.
     */
    virtual void start() = 0;

    /**
     * @brief Переносит очередь задач из старого пула потоков в текущий.
     * @param oldThreadPool Указатель на старый пул потоков.
     */
    virtual void transferTaskQueue(const std::shared_ptr<IThreadPool>& oldThreadPool) = 0;

    /**
     * @brief Создание записи.
     * @param cdr CDR запись.
     */
    virtual void writeCDR(CDR& cdr) = 0;

    /**
     * @brief Установка новой очереди задачи
     * @param task_queue новая очередь задач
     */
    virtual void setTaskQueue(std::shared_ptr<IQueue> task_queue) = 0;

    /// Мьютекс для защиты доступа к очереди задач.
    std::mutex task_queue_mutex;

    /// Очередь задач в формате пар (задача, идентификатор вызова).
    std::shared_ptr<IQueue> task_queue;
};
}

/**
 * @brief Абстрактный интерфейс для управления задачами в тредпуле.
 */
class IManager {
public:
    /**
     * @brief Конструктор класса IManager.
     * @param conf Указатель на объект конфигурации.
     * @param pool Указатель на объект тредпула.
     */
    IManager([[maybe_unused]] std::shared_ptr<utility::IConfig> conf,
             [[maybe_unused]] std::shared_ptr<TP::IThreadPool> pool) {}

    /**
     * @brief Добавляет задачу в тредпул.
     * @param number Номер телефона.
     * @return Пара, содержащая уникальный идентификатор задачи и будущий результат выполнения задачи.
     */
    virtual std::pair<TP::CallID, std::future<Result>> addTask(std::string_view number) = 0;

    /**
     * @brief Запускает выполнение задач в тредпуле.
     */
    virtual void startThreadPool() = 0;

    /**
     * @brief Останавливает выполнение задач в тредпуле.
     */
    virtual void stopThreadPool() = 0;

    /**
     * @brief Устанавливает новую конфигурацию для менеджера.
     * @param config Указатель на новый объект конфигурации.
     */
    virtual void setNewConfig(std::shared_ptr<utility::IConfig> config) = 0;

    /**
     * @brief Устанавливает новый тредпул для менеджера.
     * @param pool Указатель на новый объект тредпула.
     */
    virtual void setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) = 0;

    /**
     * @brief Выполняет обновление состояния менеджера.
     */
    virtual void update() = 0;
};
#endif // PROTEI_COV_INTERFACES_HPP
