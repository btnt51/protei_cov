#include "manager.hpp"
#include "threadpool.hpp"
#include "task.hpp"
#include <iostream>

Manager::Manager(std::shared_ptr<utility::IConfig> conf, std::shared_ptr<TP::IThreadPool> pool)  :
    IManager(conf, pool), config_(conf), threadPool_(pool) {
    std::tie(RMin_, RMax_) = config_->getMinMax();
}


std::pair<TP::CallID, std::future<Result>> Manager::addTask(std::string_view number) {
    std::shared_lock<std::shared_mutex> lc(updateMtx);
    auto now = std::chrono::system_clock::now();

    if (logger_) {
        logger_->debug("Create task with number: " + std::string{number} + " with RMin_ " + std::to_string(RMin_) +
                       " RMax_ " + std::to_string(RMax_));
    }

    auto task = std::make_shared<TP::Task>(RMin_, RMax_, number, now, logger_);
    return threadPool_->add_task(task);
}


void Manager::update() {
    std::unique_lock<std::shared_mutex> lc(updateMtx);
    std::tie(this->RMin_, this->RMax_) = config_->getMinMax();

    if(logger_)
        logger_->debug("Debug message for update: New RMin_ " + std::to_string(RMin_) + " RMax_ " + std::to_string(RMax_));

    threadPool_->task_queue->update(config_->getSizeOfQueue());
    if(static_cast<std::size_t>(config_->getAmountOfOperators()) != threadPool_->getSize()) {
        auto newThreadPool = std::make_shared<TP::ThreadPool>(config_->getAmountOfOperators(),
                                                              config_->getSizeOfQueue());
        newThreadPool->setLogger(logger_);
        setNewThreadPool(newThreadPool);
    }
}

bool Manager::processRequestForUpdate() {
    if(logger_)
        logger_->debug("Processing request for update");
    config_->updateWithRequest();
    return config_->isUpdated();
}

void Manager::setNewConfig(std::shared_ptr<utility::IConfig> config) {
    config_ = config;
    if (logger_) {
        logger_->debug("New configuration set");
        logger_->debug("Updating configuration");
    }
    update();
}

void Manager::setNewThreadPool(std::shared_ptr<TP::IThreadPool> pool) {
    stopThreadPool();
    pool->transferObjects(this->threadPool_);
    this->threadPool_ = pool;
    startThreadPool();
    if(logger_)
        logger_->debug("New thread pool set");
}

void Manager::startThreadPool() {
    threadPool_->start();
    if(logger_)
        logger_->info("Thread pool started");
}

void Manager::stopThreadPool() {
    threadPool_->stop();
    if(logger_)
        logger_->info("Thread pool stopped");
}
void Manager::setLogger(std::shared_ptr<spdlog::logger> logger) {
    this->logger_ = logger;
    logger_->info("Logger set for Manager");
}
