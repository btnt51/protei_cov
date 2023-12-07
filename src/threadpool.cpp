#include "threadpool.hpp"
#include "queue.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <random>

using namespace TP;

ThreadPool::ThreadPool(unsigned amountOfThreads, unsigned sizeOfQueue):
    IThreadPool(amountOfThreads, sizeOfQueue) {
    stopped = false;
    paused = true;
    task_queue = std::make_shared<Queue>(sizeOfQueue);
    completed_task_count = 0;
    for (unsigned int i = 0; i < amountOfThreads; i++) {
        auto* th = new Thread;
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

void ThreadPool::run(Thread* pOperator) {
    while (!stopped) {
        std::unique_lock<std::mutex> lock(task_queue_mutex);

        pOperator->is_working = false;
        tasks_access.wait(lock, [this]() -> bool { return run_allowed() || stopped; });
        pOperator->is_working = true;
        if (run_allowed()) {
            // TODO: тут должно быть лог сообщение
            auto[task, callID] = processTask();
            lock.unlock();
            executeTask(task, callID);

            if (waitForCompletion && task_queue->empty()) {
                break;
            }

            completed_task_count++;
        }
        wait_access.notify_all();
    }
}

void ThreadPool::executeTask(std::shared_ptr<ITask>& task, CallID callID) {
    try {
        auto res = task->doTask();
        task_queue->writeCDR(task->cdr);
        if(logger_)
            logger_->info("Task with CallID: " + std::to_string(callID) + " was successfully completed");
        task->promise_->set_value(res);
    } catch (...) {
        if(logger_)
            logger_->error("Task with CallID: " + std::to_string(callID) +
                           " was terminated with an exception thrown");
        task->promise_->set_exception(std::current_exception());
    }
}

std::pair<std::shared_ptr<ITask>, CallID> ThreadPool::processTask() {
    auto res = std::move(task_queue->front());
    auto threadId =  std::hash<std::thread::id>{}(std::this_thread::get_id());
    res.first->setThreadID(threadId);
    if(logger_)
        logger_->info("Task with CallID: " + std::to_string(res.second) + " in work");
    task_queue->pop();
    return res;
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

void ThreadPool::transferObjects(const std::shared_ptr<IThreadPool>& oldThreadPool) {
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
            tasks_access.notify_one();
        }
    } else {
        if(logger_)
            logger_->critical("There is no task_queue set up");
    }
    return std::make_pair(callID, std::move(future));
}


ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(task_queue_mutex);
        stopped = true;
    }
    tasks_access.notify_all();
    for (auto& thread: threads) {
        if(thread->_thread.joinable()) {
            thread->_thread.join();
        }
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


std::size_t ThreadPool::getSize() {
    return threads.size();
}
