#include "manager.hpp"
#include "threadpool.hpp"
#include <iostream>

Manager::Manager(std::shared_ptr<utility::IConfig> conf, std::shared_ptr<TP::IThreadPool> pool)  :
    IManager(conf, pool), config_(conf), threadPool_(pool) {

    std::tie(RMin_, RMax_) = config_->getMinMax();
}


std::pair<TP::CallID, std::future<Result>> Manager::addTask(std::string_view number) {
    std::shared_lock<std::shared_mutex> lc(updateMtx);
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // TODO: тут должно быть лог сообщение
    std::cout << "Create task with RMin_ " << RMin_ << " RMax_ " << RMax_ << std::endl;
    auto task = std::make_shared<TP::Task>(RMin_, RMax_, number, now);
    return threadPool_->add_task(task);
}


void Manager::update() {
    std::unique_lock<std::shared_mutex> lc(updateMtx);
    std::tie(this->RMin_, this->RMax_) = config_->getMinMax();
    std::cout << "New RMin_ " << RMin_ << " RMax_ " << RMax_ << std::endl;
    threadPool_->task_queue->update(config_->getSizeOfQueue());
    auto newThreadPool = std::make_shared<TP::ThreadPool>(config_->getAmountOfOperators(), config_->getSizeOfQueue());
    setNewThreadPool(newThreadPool);
}

void Manager::setNewConfig(std::shared_ptr<utility::IConfig> config) {
    config_ = config;
    update();
}

void Manager::setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) {
    stopThreadPool();
    pool->transferTaskQueue(this->threadPool_);
    this->threadPool_ = pool;
}

void Manager::startThreadPool() {
    threadPool_->start();
}

void Manager::stopThreadPool() {
    threadPool_->stop();
}