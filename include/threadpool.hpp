#ifndef PROTEI_COV_THREADPOOL_HPP
#define PROTEI_COV_THREADPOOL_HPP

#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>
#include <future>
#include <tuple>

#include "interfaces.hpp"
#include "recorder.hpp"

/**
 * @namespace TP
 * @breif Базовое простраснтво имён
 */
namespace TP {
/**
 * @struct Operator
 * @brief Структура, представляющая оператора ЦОВ.
 */
struct Operator {
    std::thread _thread; ///< Поток оператора.
    std::atomic<bool> is_working; ///< Флаг, указывающий, работает ли оператор.
    int operatorID; ///< ID оператора.
};


/**
 * @class ThreadPool
 * @brief Класс, реализующий пул потоков (операторов).
 *
 * @copydoc TP::IThreadPool
 */
class ThreadPool : public TP::IThreadPool, public std::enable_shared_from_this<ThreadPool> {
public:
    /**
     * @brief Конструктор.
     * @param amountOfThreads Количество потоков в пуле.
     */
    explicit ThreadPool(unsigned amountOfThreads, unsigned sizeOfQueue);

    /**
     * @brief Деструктор.
     *
     * @copydoc TP::IThreadPool::~IThreadPool
     */
    ~ThreadPool() override;

    /**
     * @copydoc TP::IThreadPool::add_task
     */
    std::pair<CallID, std::future<Result>> add_task(std::shared_ptr<ITask> task) override;

    /**
     * @brief Остановка пула.
     *
     * @copydoc TP::IThreadPool::stop
     */
    void stop() override;

    /**
     * @brief Запуск пула.
     *
     * @copydoc TP::IThreadPool::start
     */
    void start() override;

    /**
     * @brief Передача очереди задач из старого пула в текущий.
     * @param oldThreadPool Указатель на старый пул потоков.
     *
     * @copydoc TP::IThreadPool::transferObjects
     */
    void transferObjects(const std::shared_ptr<IThreadPool>& oldThreadPool) override;

    /**
     * @brief Создание записи.
     * @param cdr CDR запись.
     *
     * @copydoc TP::IThreadPool::writeCDR
     */
    void writeCDR(CDR& cdr) override;

    /**
     * @brief Установка новой очереди задачи
     * @param task_queue новая очередь задач
     *
     * @copydoc TP::IThreadPool::setTaskQueue
     */
    void setTaskQueue(std::shared_ptr<IQueue> task_queue) override;

    /**
     * @brief Устанавливает асинхронный логгер пулу потоков, а также очереди.
     * @param logger Указатель на объект логгера.
     *
     * @copydoc TP::IThreadPool::setLogger
     */
    void setLogger(std::shared_ptr<spdlog::logger> logger);

    /**
     * @brief Установка вектора разных объектов для записи cdr.
     * @param recorders массив разных объектов для записи cdr
     */
    void setRecorders(std::vector<std::shared_ptr<IRecorder>> recorders);
private:
    /**
     * @brief Мьютексы для управления доступом к различным ресурсам в пуле потоков.
     */
   // std::mutex task_queue_mutex; ///< Мьютекс для очереди задач.
    std::mutex cdr_mutex; ///< Мьютекс для операций с CDR (Call Detail Record).

    /**
     * @brief Условные переменные для управления задачами в пуле потоков.
     */
    std::condition_variable tasks_access; ///< Условная переменная для доступа к задачам.
    std::condition_variable wait_access; ///< Условная переменная для ожидания доступа.

    /**
     * @brief Вектор операторов в пуле потоков.
     */
    std::vector<Operator*> threads; ///< Вектор операторов.


    /**
     * @brief Массив выполненных задач в виде хэш-таблицы.
     */
    unsigned long long completed_task_count; ///< Количество выполненных задач.

    /**
     * @brief Очередь сигналов в пуле потоков.
     */
    std::queue<CallID> signal_queue; ///< Очередь сигналов.

    /**
     * @brief Флаг остановки работы пула.
     */
    std::atomic<bool> stopped; ///< Атомарный флаг для остановки работы пула.

    /**
     * @brief Флаг приостановки работы пула.
     */
    std::atomic<bool> paused; ///< Атомарный флаг для приостановки работы пула.
    std::atomic<bool> waitForCompletion;

    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер

    /**
     * @brief Обработка вызова в потоке оператора.
     * @param pOperator Указатель на оператора, обрабатывающего вызов.
     */
    void run(Operator* pOperator);

    /**
     * @brief Обрабатывает и подготовляет задачу из очереди пула потоков, к выполнению.
     */
    void processTask();

    /**
     * @brief Выполняет задачу и обрабатывает результат.
     * @param task Указатель на задачу, которая будет выполняться.
     */
    void executeTask(std::shared_ptr<ITask> task);

    /**
     * @brief Проверка, разрешен ли запуск нового потока в пуле.
     * @return true, если запуск разрешен, false в противном случае.
     */
    bool run_allowed() const;

    /**
     * @brief Создание уникального CallID.
     * @return Уникальный CallID.
     */
    CallID generateCallID(long long number);


};
} // namespace TP

#endif