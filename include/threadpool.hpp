#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>
#include <future>
#include <tuple>

#include "commonStructures.hpp"
#include "interfaces.hpp"
#include "recorder.hpp"

/**
 * @namespace TP
 * @breif Базовое простраснтво имён
 */
namespace TP {
class ThreadPool;

/**
 * @class Task
 * @brief Класс задачи – вызова, который обрабаытывает поток (оператор)
 */
class Task : public ITask {
public:
    /**
     * @brief конструктор
     * @param RMin нижняя граница времени
     * @param RMax верхняя граница времени
     * @param number номер звонящего
     * @param time время создания задачи
     */
    Task(int RMin, int RMax, std::string_view number, std::time_t& time);


    /// @brief Обработка вызова.
    Result doTask();

    /**
     * @brief Установка ID вызова.
     * @param id ID вызова.
     */
    void setCallID(CallID& id);

    /**
     * @brief Установка ID потока.
     * @param id ID потока.
     */
    void setThreadID(std::thread::id& id);

    /**
     * @brief Функция добавления promise в задачу
     * @param promise промис для уведомления о выполнения задачи
     */
    void addPromise(std::shared_ptr<std::promise<Result>> promise);

    std::string_view getNumber();

    /// @brief Отправка CDR на запись.
    void sendCDR();
private:
    /// @brief ID вызова.
    CallID taskId_{};
    /// @brief Верхняя граница.
    int RMin_;
    /// @brief Нижняя граница.
    int RMax_;
    /// @brief Номер вызова
    std::string_view number_;
    /// @brief Итоговый статус звонка.
    CallStatus status_;

    /// @brief CDR звонка.
    CDR cdr;




    /**
     * @brief Получение ID вызова.
     * @return ID вызова.
     */
    [[nodiscard]] CallID getCallID() const;

    /**
     * @brief Определение длительности заглушки.
     * @return Длительность заглушки.
     */
    std::chrono::seconds getDuration();


};

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
    explicit ThreadPool(unsigned amountOfThreads);

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
     * @copydoc TP::IThreadPool::transferTaskQueue
     */
    void transferTaskQueue(const std::shared_ptr<IThreadPool>& oldThreadPool) override;

    /**
     * @brief Создание записи.
     * @param cdr CDR запись.
     */
    void writeCDR(CDR& cdr);

private:
    /**
     * @brief Мьютексы для управления доступом к различным ресурсам в пуле потоков.
     */
    std::mutex task_queue_mutex; ///< Мьютекс для очереди задач.
    std::mutex cdr_mutex; ///< Мьютекс для операций с CDR (Call Detail Record).
    std::mutex log_mutex; ///< Мьютекс для записи логов.

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
     * @brief Вектор средств записи в пуле потоков.
     */
    std::vector<IRecoreder> recorders; ///< Вектор средств записи.


    /**
     * @brief Массив выполненных задач в виде хэш-таблицы.
     */
    std::unordered_map<CallID, std::shared_ptr<Task>> completed_tasks; ///< Хэш-таблица выполненных задач.
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

    /**
     * @brief Обработка вызова в потоке оператора.
     * @param pOperator Указатель на оператора, обрабатывающего вызов.
     */
    void run(Operator* pOperator);

    /**
     * @brief Проверка, разрешен ли запуск нового потока в пуле.
     * @return true, если запуск разрешен, false в противном случае.
     */
    bool run_allowed() const;

    /**
     * @brief Создание уникального CallID.
     * @return Уникальный CallID.
     */
    static CallID generateCallID();
};
} // namespace TP