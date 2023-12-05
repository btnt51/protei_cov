#include "queue.hpp"

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

bool Queue::push(std::pair<std::shared_ptr<ITask>, CallID>&& taskPair) {
    if(queue_.size() >= sizeOfQueue) {
        Result r;
        taskPair.first->cdr.status = CallStatus::Overloaded;
        taskPair.first->cdr.operatorID = 0;
        taskPair.first->cdr.callDuration = std::chrono::seconds{0};
        taskPair.first->cdr.endTime;
        taskPair.first->cdr.operatorCallTime;
        writeCDR(taskPair.first->cdr);
        r.callDuration = std::chrono::seconds{0};
        r.callID = taskPair.second;
        r.status = CallStatus::Overloaded;
        if(logger_)
            logger_->warn("Queue is overloaded. Сurrent queue size: {} while max size {}. Task with CallID {} rejected.",
                          queue_.size(), sizeOfQueue, r.callID);
        taskPair.first->promise_->set_value(r);
        return false;
    } else {
        for(auto it = queue_.begin(); it != queue_.end(); it++) {
            if(it->first->getNumber() == taskPair.first->getNumber()) {
                Result r;
                it->first->cdr.status = CallStatus::Duplication;
                it->first->cdr.operatorID = 0;
                it->first->cdr.callDuration = std::chrono::seconds{0};
                it->first->cdr.endTime;
                it->first->cdr.operatorCallTime;
                writeCDR(it->first->cdr);
                r.callDuration = std::chrono::seconds{0};
                r.callID = it->second;
                r.status = CallStatus::Duplication;
                it->first->promise_->set_value(r);
                std::rotate(it, it + 1, queue_.end());
                queue_.pop_back();

                if(logger_)
                    logger_->warn("Duplicate task with CallID {}. Removed from the queue.", r.callID);

                break;
            }
        }
        if(logger_)
            logger_->info("Task with CallID {}. Was added to the queue", taskPair.second);
        queue_.push_back(taskPair);
        if(logger_)
            logger_->info("Current queue size {}", queue_.size());
        return true;
    }
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

void Queue::writeCDR(const CDR& cdr) {
    std::lock_guard<std::mutex> lock(cdrMutex_);
    for(const auto& recorder: recorders_)
        recorder->makeRecord(cdr);
}
