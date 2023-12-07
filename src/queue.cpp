#include "queue.hpp"

/**
 * @file queue.hpp
 * @brief Содержит определение класса Queue,
 * который реализует интерфейс IQueue.
 */

using namespace TP;

Queue::Queue(int size) :
    IQueue(size), sizeOfQueue(size) {
    queue_.reserve(size);
}

std::pair<std::shared_ptr<ITask>, CallID>& Queue::back() {
    if(logger_)
        logger_->debug("Accessing the back of the queue");
    return queue_.back();
}

std::pair<std::shared_ptr<ITask>, CallID>& Queue::front() {
    if(logger_)
        logger_->debug("Accessing the front of the queue");
    return queue_.front();
}

bool Queue::empty() const {
    if(logger_)
        logger_->debug("Checking if the queue is empty");
    return queue_.empty();
}

void Queue::handleOverloadedTask(const std::pair<std::shared_ptr<ITask>, CallID>& taskPair) {
    Result r;
    auto task = taskPair.first;  // Shared ownership
    processCDR(task, false);

    r.callDuration = std::chrono::seconds{0};
    r.callID = taskPair.second;
    r.status = CallStatus::Overloaded;

    if (logger_)
        logger_->warn("Queue is overloaded. Current queue size: {} while max size {}. Task with CallID {} rejected.",
                      queue_.size(), sizeOfQueue, r.callID);

    task->promise_->set_value(r);
}

void Queue::handleTask(std::pair<std::shared_ptr<ITask>, CallID>& taskPair) {
    auto it = std::find_if(queue_.begin(), queue_.end(),
                           [taskNumber = taskPair.first->getNumber()](const auto& queuedTask) {
                               return queuedTask.first->getNumber() == taskNumber;
                           });

    if (it != queue_.end()) {
        Result r;
        auto task = it->first;
        processCDR(task, true);

        r.callDuration = std::chrono::seconds{0};
        r.callID = it->second;
        r.status = CallStatus::Duplication;

        task->promise_->set_value(r);
        queue_.erase(it);

        if (logger_)
            logger_->warn("Duplicate task with CallID {}. Removed from the queue.", r.callID);
    }

    if (logger_)
        logger_->info("Task with CallID {}. Was added to the queue", taskPair.second);

    queue_.emplace_back(std::move(taskPair));

    if (logger_)
        logger_->info("Current queue size {}", queue_.size());
}

void Queue::processCDR(std::shared_ptr<ITask> task, bool isDuplication) {
    task->cdr.status = isDuplication ? CallStatus::Duplication : CallStatus::Overloaded;
    task->cdr.operatorID = 0;
    task->cdr.callDuration = std::chrono::seconds{0};
    task->cdr.endTime = task->cdr.operatorCallTime = std::chrono::system_clock::now();
    writeCDR(task->cdr);
}

bool Queue::push(std::pair<std::shared_ptr<ITask>, CallID>&& taskPair) {
    if (queue_.size() >= sizeOfQueue) {
        handleOverloadedTask(taskPair);
        return false;
    }

    handleTask(taskPair);
    return true;
}

void Queue::pop() {
    if(logger_)
        logger_->debug("Removing the front of the queue");
    queue_.erase(queue_.cbegin());
}

void Queue::update(int size) {
    if(logger_)
        logger_->info("Queue size updated to {}", size);
    sizeOfQueue = size;
}

void Queue::setLogger(std::shared_ptr<spdlog::logger> logger) {
    this->logger_ = logger;
}
void Queue::setRecorders(std::vector<std::shared_ptr<IRecorder>> recorders) {
    recorders_ = recorders;
}

void Queue::writeCDR(const CDR& cdr){
    std::lock_guard<std::mutex> lock(cdrMutex_);
    for(const auto& recorder: recorders_)
        recorder->makeRecord(cdr);
}
