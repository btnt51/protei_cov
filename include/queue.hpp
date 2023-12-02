#ifndef PROTEI_COV_QUEUE_HPP
#define PROTEI_COV_QUEUE_HPP
#include <memory>
#include <queue>
#include <tuple>

#include "commonStructures.hpp"
#include "interfaces.hpp"
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

private:
    std::vector<std::pair<std::shared_ptr<ITask>, CallID>> queue_; ///< Вектор для хранения задач в очереди.
    std::size_t sizeOfQueue; ///< Максимальный размер очереди.
    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер
};
}
#endif // PROTEI_COV_QUEUE_HPP
