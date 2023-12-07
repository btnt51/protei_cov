#include <random>

#include "task.hpp"

using namespace TP;
using std::string_literals::operator""s;

Task::Task(int RMin, int RMax, std::string_view number, const std::chrono::system_clock::time_point& startTime, std::shared_ptr<spdlog::logger> logger)
    : ITask(RMin, RMax, number, startTime, logger), RMin_(RMin), RMax_(RMax), number_(number), logger_(logger) {
    if(logger_) {
        logger_->info("Creating Task with RMin: {} RMax: {}", std::to_string(RMin_), std::to_string(RMax_));
        logger_->debug("Task initialized with number: {}", std::string{number_});
    }
    cdr.startTime = startTime;
    cdr.number = number;
    promise_ = std::make_shared<std::promise<Result>>();
}

void Task::setCallID(TP::CallID& id) {
    this->taskId_ = id;
    cdr.callID = id;
    logger_->info("Setting CallID to task with number {}: {} ", std::string{number_}, std::to_string(id));
}


void Task::setThreadID(std::size_t& id) {
    cdr.operatorID = id;
    logger_->info("Setting ThreadID to task with number {}: {}", std::string{number_}, std::to_string(id));
}

void Task::addPromise(std::shared_ptr<std::promise<Result>> t) {
    promise_ = t;
    logger_->debug("Added promise for Task");
}


CallID Task::getCallID() const {
    return this->taskId_;
}

std::string_view Task::getNumber() {
    return this->number_;
}

std::chrono::seconds Task::getDuration() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(RMin_, RMax_);
    auto temp = distrib(gen);
    std::chrono::seconds randomSeconds(temp);
    return randomSeconds;
}


void Task::setCdrValues(const std::chrono::seconds& timeDiff) {
    if(logger_) {
        logger_->info("Setting cdr values for call with number: {} and callID: {}", number_, taskId_);
    }
    cdr.operatorCallTime = std::chrono::system_clock::now();
    cdr.callDuration = (timeDiff.count() < RMax_) ? getDuration() : std::chrono::seconds{0};
    cdr.status = (timeDiff.count() < RMax_) ? CallStatus::Completed : CallStatus::Timeout;

}

void Task::logCallDetails() {
    if (logger_) {
        logger_->info("Call with number: {} {} for {} seconds",
                      cdr.number, (cdr.status == CallStatus::Completed) ? "will sleep" : "was timed out",
                      cdr.callDuration.count());
        logger_->debug("Sleeping for {} seconds", cdr.callDuration.count());
        if (cdr.status == CallStatus::Completed)
            std::this_thread::sleep_for(cdr.callDuration);
    }

    cdr.endTime = std::chrono::system_clock::now();

    logger_->debug("Call with number {} and callID {} {}",
                   cdr.number, taskId_, (cdr.status == CallStatus::Completed) ? "completed successfully" : "timed out");
}

Result Task::createResultObject() const {
    if(logger_)
        logger_->info("Filling result variable");
    Result r;
    r.status = cdr.status;
    r.callDuration = cdr.callDuration;
    r.callID = taskId_;
    return r;
}


Result Task::doTask() {
    try {
        auto now = std::chrono::system_clock::now();
        auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - cdr.startTime);

        setCdrValues(timeDiff);
        logCallDetails();

        logger_->info("Writing CDR for task with number: {}", cdr.number);

        return createResultObject();
    } catch (const std::exception& e) {
        logger_->error("Exception in doTask: {}", e.what());
        promise_->set_exception(std::current_exception());
        throw;
    }
}
