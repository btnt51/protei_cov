#include "threadpool.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <random>


using namespace TP;

Task::Task(int RMin, int RMax, std::string_view number, std::time_t& startTime) : ITask(RMin, RMax, number, startTime),
    RMin_(RMin), RMax_(RMax), number_(number) {
    std::cout << "From Task() RMin:" << RMin_ << " RMax: " << RMax_ << std::endl;
    cdr.startTime = startTime;
    cdr.number = number;
    status_ = CallStatus::awaiting;
    pool_ = nullptr;
    promise_ = std::make_shared<std::promise<Result>>();
}

void Task::setCallID(TP::CallID& id) {
    this->taskId_ = id;
    cdr.callID = id;
}


void Task::setThreadID(std::thread::id& id) {
    cdr.operatorID = id;
}

void Task::addPromise(std::shared_ptr<std::promise<Result>> t) {
    promise_ = t;
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
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (now - cdr.startTime < RMax_) {
            // TODO: тут должно быть лог сообщение
            std::chrono::seconds randomSeconds = getDuration();
            cdr.callDuration = randomSeconds;
            std::cout << "Call with number: " << cdr.number << " will sleep for " << randomSeconds.count() << "s" << std::endl;
            std::this_thread::sleep_for(randomSeconds);
            cdr.endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            status_ = CallStatus::completed;
            cdr.status = CallStatus::completed;
            sendCDR();
            Result r;
            r.status = CallStatus::completed;
            r.callDuration = randomSeconds;
            r.callID = taskId_;
            // TODO: тут должно быть лог сообщение
            return r;
        } else {
            // TODO: тут должно быть лог сообщение
            cdr.endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            cdr.callDuration = std::chrono::seconds{0};
            cdr.status = CallStatus::rejected;
            sendCDR();
            Result r;
            r.status = CallStatus::rejected;
            r.callDuration = std::chrono::seconds{0};
            r.callID = taskId_;
            return r;
        }
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        this->promise_->set_exception(std::current_exception());
        throw e;
    }
}


ThreadPool::ThreadPool(unsigned amountOfThreads):
    IThreadPool(amountOfThreads) {
    stopped = false;
    paused = true;

    completed_task_count = 0;
    for (unsigned int i = 0; i < amountOfThreads; i++) {
        auto* th = new Operator;
        th->_thread = std::thread{&ThreadPool::run, this, th};
        th->is_working = false;
        threads.push_back(th);
    }
}

bool ThreadPool::run_allowed() const {
    return (!task_queue.empty() && !paused);
}

void ThreadPool::run(Operator* pOperator) {
    while (!stopped) {
        std::unique_lock<std::mutex> lock(task_queue_mutex);

        pOperator->is_working = false;
        tasks_access.wait(lock, [this]() -> bool { return run_allowed() || stopped; });
        pOperator->is_working = true;
        if (run_allowed()) {
            // TODO: тут должно быть лог сообщение
            auto [elem, callID] = std::move(task_queue.front());
            task_queue.pop();
            lock.unlock();
            try {
                auto res = elem->doTask();
                elem->promise_->set_value(res);
            } catch (...) {
                elem->promise_->set_exception(std::current_exception());
            }
            if (waitForCompletion && task_queue.empty()) {
                break;
            }

            completed_task_count++;
        }
        wait_access.notify_all();
    }
}

void ThreadPool::start() {
    if (paused) {
        stopped = false;
        paused = false;
        waitForCompletion = false;
        tasks_access.notify_all();
    }
}

void ThreadPool::stop() {
    paused = true;
    waitForCompletion = true;
}

void ThreadPool::transferTaskQueue(const std::shared_ptr<IThreadPool>& oldThreadPool) {
    std::lock_guard<std::mutex> oldPoolLock(oldThreadPool->task_queue_mutex);
    std::lock_guard<std::mutex> thisPoolLock(task_queue_mutex);

    if(this != oldThreadPool.get()) {
        this->task_queue = std::move(oldThreadPool->task_queue);
    }
}

std::pair<CallID, std::future<Result>> ThreadPool::add_task(std::shared_ptr<ITask> task) {
    // TODO: тут должно быть лог сообщение
    std::lock_guard<std::mutex> lock(task_queue_mutex);
    auto callID = generateCallID();
    auto future = task->promise_->get_future();
    task_queue.push(std::make_pair(task, callID));
    task_queue.back().first->pool_ = shared_from_this();
    tasks_access.notify_one();
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

CallID ThreadPool::generateCallID() {
    return CallID{0};
}

