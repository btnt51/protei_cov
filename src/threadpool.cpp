#include "threadpool.hpp"
#include "queue.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <random>

using namespace TP;
using std::string_literals::operator""s;

Task::Task(int RMin, int RMax, std::string_view number, std::time_t& startTime, std::shared_ptr<spdlog::logger> logger)
    : ITask(RMin, RMax, number, startTime, logger), RMin_(RMin), RMax_(RMax), number_(number), logger_(logger) {
    if(logger_) {
        logger_->info("Creating Task with RMin: {} RMax: {}", std::to_string(RMin_), std::to_string(RMax_));
        logger_->debug("Task initialized with number: {}", std::string{number_});
    }
    cdr.startTime = startTime;
    cdr.number = number;
    status_ = CallStatus::awaiting;
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
        if (now - cdr.startTime < RMax_) {
            std::chrono::seconds randomSeconds = getDuration();
            cdr.callDuration = randomSeconds;
            logger_->info("Call with number: {} will sleep for {} seconds", std::string{cdr.number},
                          std::to_string(randomSeconds.count()));
            logger_->debug("Sleeping for {} seconds", std::to_string(randomSeconds.count()));
            std::this_thread::sleep_for(randomSeconds);
            cdr.endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            status_ = CallStatus::completed;
            cdr.status = CallStatus::completed;
            logger_->info("Writing CDR for task with number:"s + std::string{cdr.number});
            sendCDR();
            logger_->info("Filling result variable");
            logger_->debug("Call with number {}  and callID {} completed successfully", std::string{cdr.number}, std::to_string(taskId_));
            r.status = CallStatus::completed;
            r.callDuration = randomSeconds;
            r.callID = taskId_;
            // TODO: тут должно быть лог сообщение
        } else {
            logger_->info("Call with number: "s + std::string{cdr.number} + " was timed out"s);
            cdr.endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            cdr.callDuration = std::chrono::seconds{0};
            cdr.status = CallStatus::timeout;
            logger_->info("Writing CDR for task with number: {}", std::string{cdr.number});

            sendCDR();
            logger_->info("Filling result variable");
            logger_->debug("Call with number {}  and callID {} timed out", std::string{cdr.number},
                           std::to_string(taskId_));
            r.status = CallStatus::timeout;
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


ThreadPool::ThreadPool(unsigned amountOfThreads, unsigned sizeOfQueue):
    IThreadPool(amountOfThreads, sizeOfQueue) {
    stopped = false;
    paused = true;
    task_queue = std::make_shared<Queue>(sizeOfQueue);
    completed_task_count = 0;
    for (unsigned int i = 0; i < amountOfThreads; i++) {
        auto* th = new Operator;
        th->_thread = std::thread{&ThreadPool::run, this, th};
        th->is_working = false;
        threads.push_back(th);
    }
}

bool ThreadPool::run_allowed() const {
    if(task_queue)
        return (!task_queue->empty() && !paused);
    if(logger_)
        logger_->critical("There is no task_queue set up");
    return false;
}

void ThreadPool::run(Operator* pOperator) {
    while (!stopped) {
        std::unique_lock<std::mutex> lock(task_queue_mutex);

        pOperator->is_working = false;
        tasks_access.wait(lock, [this]() -> bool { return run_allowed() || stopped; });
        pOperator->is_working = true;
        if (run_allowed()) {
            // TODO: тут должно быть лог сообщение
            auto [elem, callID] = std::move(task_queue->front());
            auto threadId =  std::hash<std::thread::id>{}(std::this_thread::get_id());
            elem->setThreadID(threadId);
            if(logger_)
                logger_->info("Task with CallID: " + std::to_string(callID) + " in work");
            task_queue->pop();
            lock.unlock();
            try {
                auto res = elem->doTask();
                if(logger_)
                    logger_->info("Task with CallID: " + std::to_string(callID) + " was successfully completed");
                elem->promise_->set_value(res);
            } catch (...) {
                if(logger_)
                    logger_->error("Task with CallID: " + std::to_string(callID) +
                                   " was terminated with an exception thrown");
                elem->promise_->set_exception(std::current_exception());
            }

            if (waitForCompletion && task_queue->empty()) {
                break;
            }

            completed_task_count++;
        }
        wait_access.notify_all();
    }
}

void ThreadPool::start() {
    if(logger_)
        logger_->info("Starting threadpool");
    if (paused||stopped) {
        stopped = false;
        paused = false;
        waitForCompletion = false;
        tasks_access.notify_all();
    }
}

void ThreadPool::stop() {
    if(logger_)
        logger_->info("Stopping thread pool");
    paused = true;
    waitForCompletion = true;
}

void ThreadPool::transferTaskQueue(const std::shared_ptr<IThreadPool>& oldThreadPool) {
    std::lock_guard<std::mutex> oldPoolLock(oldThreadPool->task_queue_mutex);
    std::lock_guard<std::mutex> thisPoolLock(task_queue_mutex);
    if(logger_)
        logger_->info("Transfering code task queue");
    if(this != oldThreadPool.get()) {
        this->task_queue = oldThreadPool->task_queue;
    }
}

std::pair<CallID, std::future<Result>> ThreadPool::add_task(std::shared_ptr<ITask> task) {
    // TODO: тут должно быть лог сообщение
    std::lock_guard<std::mutex> lock(task_queue_mutex);
    auto callID = generateCallID(std::stoll(std::string{task->getNumber()}));
    task->setCallID(callID);
    auto future = task->promise_->get_future();
    if(task_queue) {
        if(task_queue->push(std::make_pair(task, callID))) {
            task_queue->back().first->pool_ = shared_from_this();
            tasks_access.notify_one();
        }
    } else {
        if(logger_)
            logger_->critical("There is no task_queue set up");
    }
    return std::make_pair(callID, std::move(future));
}


void ThreadPool::writeCDR(CDR& cdr) {
    std::lock_guard<std::mutex> lg(cdr_mutex);
    for (auto& recorder: recorders)
        recorder.makeRecord(cdr);
}

ThreadPool::~ThreadPool() {
    stopped = true;
    tasks_access.notify_all();
    for (auto& thread: threads) {
        thread->_thread.join();
        delete thread;
    }
}

void ThreadPool::setTaskQueue(std::shared_ptr<IQueue> task_queue) {
    std::lock_guard<std::mutex> thisPoolLock(task_queue_mutex);
    if(logger_)
        logger_->info("Thread pool set up task queue");
    this->task_queue = task_queue;
}

CallID ThreadPool::generateCallID(long long number) {
    auto threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
    CallID res{threadID+number};
    if(logger_)
        logger_->info("Generated CallID: " + std::to_string(res) + " for call with number: " + std::to_string(number));
    return res;
}

void ThreadPool::setLogger(std::shared_ptr<spdlog::logger> logger) {
    this->logger_ = logger;
    if(logger_) {
        task_queue->setLogger(logger);
        logger_->info("ThreadPoll set up logger");
    }
}
