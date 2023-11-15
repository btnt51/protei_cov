#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "commonStructures.hpp"
#include "recorder.hpp"

// TP - thread pool namespace
namespace TP {
class ThreadPool;
class Task {
public:
    //конструктор
    Task(int RMin, int RMax, long long number, std::time_t&);

    // уведомление оператора
    void sendSignalToThread();

    // обработка вызова
    void doTask();

    // сохранение id вызова
    void setCallID(CallID& id);

    // запись номер оператора
    void setThreadID(std::thread::id& id);

private:
    // id вызова
    CallID taskId_{};
    // верхняя граница
    int RMin_;
    // нижняя граница
    int RMax_;
    // номер вызова
    long long number_;
    // итоговый статус звонка
    CallStatus status_;
    // пул потоков
    ThreadPool* pool_{};
    //CDR звонка
    CDR cdr;

    // оптравка CDR на запись
    void sendCDR();

    // получение id вызова
    [[nodiscard]] CallID getCallID() const;

    // пределение заглушки
    std::chrono::seconds getDuration();

    // дружественный класс
    friend class ThreadPool;
};

// представление оператора цода
struct Operator {
    std::thread _thread;
    std::atomic<bool> is_working;
    int operatorID;
};

class ThreadPool {
public:
    //конструктор
    explicit ThreadPool(unsigned amountOfThreads);

    //деструктор
    ~ThreadPool();

    // добавление задачи
    CallID add_task(const Task& task);

    // ожидание сигнала
    CallID wait_signal();

    // остановка пула
    void wait();

    // остановка пула
    void stop();

    // запуск пула
    void start();

    // получения запроса
    std::shared_ptr<Task> get_result(CallID id);

    // очистка выполненых
    void clear_completed();

    // обновление пула
    void updateThreadPool(int amountOfThreads);

private:
    // мьютексы для очереди
    std::mutex task_queue_mutex;
    std::mutex completed_tasks_mutex;
    std::mutex signal_queue_mutex;

    // служебные мьютексы
    std::mutex cdr_mutex;
    std::mutex log_mutex;
    std::mutex wait_mutex;

    // условные переменные для управления задачами
    std::condition_variable tasks_access;
    std::condition_variable wait_access;

    // вектор операторов
    std::vector<Operator*> threads;

    // вектор средстав записи
    std::vector<IRecoreder> recorders;

    //очередь задач
    std::queue<std::pair<std::shared_ptr<Task>, CallID>> task_queue;

    // массив выполненных задач в виде хэш-таблицы
    std::unordered_map<CallID, std::shared_ptr<Task>> completed_tasks;
    unsigned long long completed_task_count;

    std::atomic<CallID> last_callID_in_work;
    //очередь сигналов
    std::queue<CallID> signal_queue;

    // флаг остановки работы пула
    std::atomic<bool> stopped;

    // флаг приостановки работы
    std::atomic<bool> paused;
    std::atomic<bool> ignore_signals;

    // обработка вызова
    void run(Operator* pOperator);

    // обработка сигнала
    void receive_signal(CallID id);

    // разрешение запуска очередного потока
    bool run_allowed() const;

    // проверка выполнения всех задач из очереди
    bool is_completed() const;

    // проверка, занятости хотя бы одного потока
    bool is_standby() const;

    // создание уникального CallID
    static CallID generateCallID();

    // создание записи
    void writeCDR(CDR& cdr);

    // дружественные функции для работы с пуллом
    friend void Task::sendSignalToThread();
    friend void Task::sendCDR();
};
} // namespace TP