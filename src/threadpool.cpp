#include "threadpool.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <random>


using namespace TP;

Task::Task(int RMin, int RMax, std::string_view number, std::time_t& startTime): RMin_(RMin), RMax_(RMax) {
    cdr.startTime = startTime;
    cdr.number = number;
    status_ = CallStatus::awaiting;
    pool_ = nullptr;
}

void Task::setCallID(TP::CallID& id) {
    this->taskId_ = id;
    cdr.callID = id;
}


void Task::setThreadID(std::thread::id& id) {
    cdr.operatorID = id;
}

void Task::addPromise(std::shared_ptr<std::promise<Result>> t) {
    promise_ = std::move(t);
}


void Task::sendCDR() {
    pool_->writeCDR(cdr);
}

CallID Task::getCallID() const {
    return this->taskId_;
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
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (now - cdr.startTime < RMax_) {
        // TODO: тут должно быть лог сообщение
        std::chrono::seconds randomSeconds = getDuration();
        cdr.callDuration = randomSeconds;
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
}


ThreadPool::ThreadPool(unsigned amountOfThreads) {
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
            last_callID_in_work = callID;
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
        paused = false;
        waitForCompletion = false;
        tasks_access.notify_all();
    }
}

void ThreadPool::stop() {
    paused = true;
    waitForCompletion = true;
}

std::pair<CallID, std::future<Result>> ThreadPool::add_task(const Task& task) {
    // TODO: тут должно быть лог сообщение
    std::lock_guard<std::mutex> lock(task_queue_mutex);
    auto callID = generateCallID();;
    auto future = task.promise_->get_future();
    task_queue.push(std::make_pair(std::make_shared<Task>(task), callID));
    task_queue.back().first->pool_ = this;
    tasks_access.notify_one();
    return {callID, std::move(future)};
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
