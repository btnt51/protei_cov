#ifndef PROTEI_COV_TASK_HPP
#define PROTEI_COV_TASK_HPP
#include "commonStructures.hpp"
#include "interfaces.hpp"
namespace TP {
class ThreadPool;

/**
 * @class Task
 * @brief Класс задачи – вызова, который обрабаытывает поток (оператор)
 */
class Task: public ITask {
public:
    /**
     * @brief конструктор
     * @param RMin нижняя граница времени
     * @param RMax верхняя граница времени
     * @param number номер звонящего
     * @param time время создания задачи
     */
    Task(int RMin,
         int RMax,
         std::string_view number,
         const std::chrono::system_clock::time_point& startTime,
         std::shared_ptr<spdlog::logger> logger);

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
    void setThreadID(std::size_t& id);

    /**
     * @brief Функция добавления promise в задачу
     * @param promise промис для уведомления о выполнения задачи
     */
    void addPromise(std::shared_ptr<std::promise<Result>> promise);

    /**
     * @brief Отправляет CDR на запись.
     */
    void sendCDR();

    /**
     * @brief Функция получения номер звонящего
     * @return возвращает number_
     */
    std::string_view getNumber();

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
    std::shared_ptr<spdlog::logger> logger_;

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
}
#endif // PROTEI_COV_TASK_HPP
