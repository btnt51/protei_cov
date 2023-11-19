#include "threadpool.hpp"
#include "commonStructures.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <random>

// #include <boost/uuid/>

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

void Task::sendSignalToThread() {
    pool_->receive_signal(taskId_);
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

void Task::doTask() {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (now - cdr.startTime > RMax_) {
        std::chrono::seconds randomSeconds = getDuration();
        cdr.callDuration = randomSeconds;
        std::this_thread::sleep_for(randomSeconds);
        std::cout << "InTime" << std::endl;
        cdr.endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        status_ = CallStatus::completed;
        cdr.status = CallStatus::completed;
        sendCDR();
    } else {
        cdr.endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        cdr.callDuration = std::chrono::seconds{0};
        cdr.status = CallStatus::rejected;
        sendCDR();
    }
}


ThreadPool::ThreadPool(unsigned amountOfThreads) {
    stopped = false;
    paused = true;

    completed_task_count = 0;
    ignore_signals = true;
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

            auto [elem, callID] = std::move(task_queue.front());
            task_queue.pop();
            last_callID_in_work = callID;
            lock.unlock();
            elem->doTask();

            std::lock_guard<std::mutex> lg(completed_tasks_mutex);
            completed_tasks[callID] = elem;
            completed_task_count++;
        }
        wait_access.notify_all();
    }
}

void ThreadPool::start() {
    if (paused) {
        paused = false;
        tasks_access.notify_all();
    }
}

void ThreadPool::stop() {
    paused = true;
}

void ThreadPool::receive_signal(CallID id) {
    std::lock_guard<std::mutex> lock(signal_queue_mutex);
    signal_queue.emplace(id);
    if (!ignore_signals)
        stop();
}

CallID ThreadPool::add_task(const Task& task) {
    std::lock_guard<std::mutex> lock(task_queue_mutex);
    auto callID = generateCallID();;
    task_queue.push(std::make_pair(std::make_shared<Task>(task), callID));
    task_queue.back().first->pool_ = this;
    tasks_access.notify_one();
    return callID;
}

std::shared_ptr<Task> ThreadPool::get_result(CallID id) {
    auto elem = completed_tasks.find(id);
    if (elem != completed_tasks.end())
        return std::reinterpret_pointer_cast<Task>(elem->second);
    else
        return nullptr;
}

bool ThreadPool::is_completed() const {
    return completed_task_count == static_cast<unsigned long long>(last_callID_in_work);
}

bool ThreadPool::is_standby() const {
    if (!paused)
        return false;
    return std::all_of(threads.begin(), threads.end(), [](const Operator* thread) { return !thread->is_working; });
}

void ThreadPool::wait() {
    std::lock_guard<std::mutex> lock_wait(wait_mutex);

    start();

    std::unique_lock<std::mutex> lock(task_queue_mutex);
    wait_access.wait(lock, [this]() -> bool { return is_completed(); });

    stop();
}

CallID ThreadPool::wait_signal() {
    std::lock_guard<std::mutex> lock_wait(wait_mutex);

    ignore_signals = false;

    signal_queue_mutex.lock();
    if (signal_queue.empty())
        start();
    else
        stop();
    signal_queue_mutex.unlock();

    std::unique_lock<std::mutex> lock(task_queue_mutex);
    wait_access.wait(lock, [this]() -> bool { return is_completed() || is_standby(); });

    ignore_signals = true;

    std::lock_guard<std::mutex> lock_signals(signal_queue_mutex);
    if (signal_queue.empty())
        return 0;
    else {
        CallID signal = signal_queue.front();
        signal_queue.pop();
        return signal;
    }
}

void ThreadPool::clear_completed() {
    std::scoped_lock lock(completed_tasks_mutex, signal_queue_mutex);
    completed_tasks.clear();
    while (!signal_queue.empty())
        signal_queue.pop();
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

void ThreadPool::updateThreadPool(int amountOfThreads) {
    amountOfThreads -= amountOfThreads;
}

CallID ThreadPool::generateCallID() {
    return CallID{0};
}
