#include "queue.hpp"

using namespace TP;

Queue::Queue(int size) :
    IQueue(size), sizeOfQueue(size) {
    queue_.reserve(size);
}

std::pair<std::shared_ptr<ITask>, CallID> Queue::back() {
    return queue_.back();
}

std::pair<std::shared_ptr<ITask>, CallID> Queue::front() {
    return queue_.front();
}

bool Queue::empty() const {
    return queue_.empty();
}

bool Queue::push(std::pair<std::shared_ptr<ITask>, CallID>&& taskPair) {
    for(auto it = queue_.begin(); it != queue_.end(); it++) {
        if(it->first->getNumber() == taskPair.first->getNumber()) {
            std::rotate(it, it + 1, queue_.end());
            Result r;
            r.callDuration = std::chrono::seconds{0};
            r.callID = it->second;
            r.status = CallStatus::Duplication;
            it->first->promise_->set_value(r);
            break;
        }
    }
    if(queue_.size() < sizeOfQueue) {
        queue_.push_back(taskPair);
        return true;
    } else {
        return false;
    }
}

void Queue::pop() {
    queue_.erase(queue_.cbegin());
}

void Queue::update(int size) {
    sizeOfQueue = size;
}