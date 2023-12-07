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

    /**
     * @brief Устанавливает значения CDR (Call Detail Record) на основе временной разницы.
     * @param timeDiff Временная разница, используемая для установки значений CDR.
     */
    void setCdrValues(const std::chrono::seconds& timeDiff);

    /**
     * @brief логирование информации о вызове
     */
    void logCallDetails();

    /**
     * @brief Создает объект Result на основе текущих данных.
     * @return Объект Result, представляющий результат вызова.
     */
    Result createResultObject() const;
};
}
#endif // PROTEI_COV_TASK_HPP
