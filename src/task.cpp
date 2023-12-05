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
    status_ = CallStatus::Awaiting;
    pool_ = nullptr;
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


void Task::sendCDR() {
    if(pool_)
        pool_->writeCDR(cdr);
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

Result Task::doTask() {
    try {
        Result r;
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (now - std::chrono::system_clock::to_time_t(cdr.startTime) < RMax_) {
            cdr.operatorCallTime = std::chrono::system_clock::now();
            std::chrono::seconds randomSeconds = getDuration();
            cdr.callDuration = randomSeconds;
            if(logger_) {
                logger_->info("Call with number: {} will sleep for {} seconds",
                              std::string{cdr.number},
                              std::to_string(randomSeconds.count()));
                logger_->debug("Sleeping for {} seconds", std::to_string(randomSeconds.count()));
                std::this_thread::sleep_for(randomSeconds);
            }
            cdr.endTime = std::chrono::system_clock::now();
            status_ = CallStatus::Completed;
            cdr.status = CallStatus::Completed;
            logger_->info("Writing CDR for task with number:"s + std::string{cdr.number});
            sendCDR();
            logger_->info("Filling result variable");
            logger_->debug("Call with number {}  and callID {} completed successfully", std::string{cdr.number}, std::to_string(taskId_));
            r.status = CallStatus::Completed;
            r.callDuration = randomSeconds;
            r.callID = taskId_;
            // TODO: тут должно быть лог сообщение
        } else {
            logger_->info("Call with number: "s + std::string{cdr.number} + " was timed out"s);
            cdr.operatorCallTime = std::chrono::system_clock::now();
            cdr.callDuration = std::chrono::seconds{0};
            cdr.status = CallStatus::Timeout;
            logger_->info("Writing CDR for task with number: {}", std::string{cdr.number});

            sendCDR();
            logger_->info("Filling result variable");
            logger_->debug("Call with number {}  and callID {} timed out", std::string{cdr.number},
                           std::to_string(taskId_));
            r.status = CallStatus::Timeout;
            r.callDuration = std::chrono::seconds{0};
            r.callID = taskId_;

        }
        return r;
    } catch(std::exception& e) {
        logger_->error("Exception in doTask: {}", e.what());
        this->promise_->set_exception(std::current_exception());
        throw e;
    }
}