#ifndef PROTEI_COV_QUEUE_HPP
#define PROTEI_COV_QUEUE_HPP
#include <memory>
#include <queue>
#include <tuple>

#include "commonStructures.hpp"
#include "interfaces.hpp"
#include "recorder.hpp"
namespace TP {
/**
 * @brief Класс Queue представляет собой реализацию интерфейса IQueue.
 *
 * @copydoc IQueue
 */
class Queue : public IQueue {
public:
    /**
     * @brief Конструктор класса Queue.
     * @param size Максимальный размер очереди.
     */
    explicit Queue(int size);

    /**
     * @brief Возвращает ссылку на задачу в конце очереди.
     * @return Ссылка на пару, содержащую задачу и ее уникальный идентификатор вызова.
     *
     * @copydoc IQueue::back
     */
    std::pair<std::shared_ptr<ITask>, CallID>& back() override;

    /**
     * @brief Возвращает ссылку на задачу в начале очереди.
     * @return Ссылка на пару, содержащую задачу и ее уникальный идентификатор вызова.
     *
     * @copydoc IQueue::front
     */
    std::pair<std::shared_ptr<ITask>, CallID>& front() override;

    /**
     * @brief Проверяет, является ли очередь пустой.
     * @return true, если очередь пуста, false в противном случае.
     *
     * @copydoc IQueue::empty
     */
    [[nodiscard]] bool empty() const override;

    /**
     * @brief Добавляет задачу в конец очереди.
     * @param taskPair Пара, содержащая задачу и ее уникальный идентификатор вызова.
     * @return true, если задача успешно добавлена, false в противном случае.
     *
     * @copydoc IQueue::push
     */
    [[nodiscard]] bool push(std::pair<std::shared_ptr<ITask>, CallID>&& taskPair) override;

    /**
     * @brief Удаляет задачу из начала очереди.
     *
     * @copydoc IQueue::pop
     */
    void pop() override;

    /**
     * @brief Обновляет максимальный размер очереди.
     * @param size Новый максимальный размер очереди.
     *
     * @copydoc IQueue::update
     */
    void update(int size) override;

    /**
     * @brief Устанавливает асинхронный логгер.
     * @param logger Указатель на объект логгера.
     */
    void setLogger(std::shared_ptr<spdlog::logger> logger) override;

    /**
     * @brief Установка вектора разных объектов для записи cdr.
     * @param recorders массив разных объектов для записи cdr
     */
    void setRecorders(std::vector<std::shared_ptr<IRecorder>> recorders) override;

    void writeCDR(const CDR& cdr) override;

private:
    /**
     * @brief Обрабатывает задачу, когда очередь перегружена.
     * @param taskPair Пара, содержащая указатель на задачу и идентификатор вызова.
     */
    void handleOverloadedTask(const std::pair<std::shared_ptr<ITask>, CallID>& taskPair);

    /**
     * @brief Обрабатывает задачу и проверяет на дубликат.
     * @param taskPair Пара, содержащая указатель на задачу и идентификатор вызова.
     */
    void handleTask(std::pair<std::shared_ptr<ITask>, CallID>& taskPair);

    /**
     * @brief Дополняет CDR и отправляет его на запись
     * @param task Ссылка на задачу для которой будет обрабатываться cdr
     * @param isDuplication тип статуса, если true,
     * значит в CDR будет записано CallStatus::Duplication, иначе будет
     * записано CallStatus::Overloaded
     */
    void processCDR(std::shared_ptr<ITask> task, bool isDuplication);

    std::vector<std::pair<std::shared_ptr<ITask>, CallID>> queue_; ///< Вектор для хранения задач в очереди.
    std::size_t sizeOfQueue; ///< Максимальный размер очереди.
    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер
    std::vector<std::shared_ptr<IRecorder>> recorders_;
    std::mutex cdrMutex_;
};
}
#endif // PROTEI_COV_QUEUE_HPP
