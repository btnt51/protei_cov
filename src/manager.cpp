
#include "manager.hpp"
#include "threadpool.hpp"
#include <iostream>
#include <memory>
#include <utility>

Manager::Manager(std::shared_ptr<utility::IConfig> conf, std::shared_ptr<TP::IThreadPool> pool)  :
    IManager(conf, pool), config_(conf), threadPool_(pool) {

    std::tie(RMin_, RMax_) = config_->getMinMax();
}


std::pair<TP::CallID, std::future<Result>> Manager::addTask(std::string_view number) {
    std::shared_lock<std::shared_mutex> lc(updateMtx);
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // TODO: тут должно быть лог сообщение
    std::cout << "Create task with RMin_ " << RMin_ << " RMax_ " << RMax_ << std::endl;
    TP::Task task = TP::Task(RMin_, RMax_, number, now);
    return threadPool_->add_task(task);
}


void Manager::update() {
    std::unique_lock<std::shared_mutex> lc(updateMtx);
    std::tie(this->RMin_, this->RMax_) = config_->getMinMax();
    std::cout << "New RMin_ " << RMin_ << " RMax_ " << RMax_ << std::endl;
    auto newThreadPool = std::make_shared<TP::ThreadPool>(config_->getAmountOfOperators());
    this->threadPool_->stop();
    newThreadPool->transferTaskQueue(this->threadPool_);
    this->threadPool_ = std::move(newThreadPool);
    this->threadPool_->start();
}

void Manager::setNewConfig(std::shared_ptr<utility::IConfig> config) {
    config_ = config;
}

void Manager::setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) {
    threadPool_ = pool;
}

void Manager::startThreadPool() {
    threadPool_->start();
}

void Manager::stopThreadPool() {
    threadPool_->stop();
}