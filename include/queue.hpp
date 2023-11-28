#ifndef PROTEI_COV_QUEUE_HPP
#define PROTEI_COV_QUEUE_HPP
#include <memory>
#include <queue>
#include <tuple>

#include "commonStructures.hpp"
#include "interfaces.hpp"
namespace TP {
class Queue : public IQueue {
public:
    explicit Queue(int size);

    std::pair<std::shared_ptr<ITask>, CallID>& back() override;

    std::pair<std::shared_ptr<ITask>, CallID>& front() override;

    [[nodiscard]] bool empty() const override;

    [[nodiscard]] bool push(std::pair<std::shared_ptr<ITask>, CallID>&& taskPair) override;

    void pop() override;

    void update(int size) override;

    /**
     * @brief Устанавливает асинхронный логгер.
     * @param logger Указатель на объект логгера.
     */
    void setLogger(std::shared_ptr<spdlog::logger> logger) override;

private:
    std::vector<std::pair<std::shared_ptr<ITask>, CallID>> queue_;
    std::size_t sizeOfQueue;
    std::shared_ptr<spdlog::logger> logger_; ///< указатель на асинхронный логгер
};
}
#endif // PROTEI_COV_QUEUE_HPP
