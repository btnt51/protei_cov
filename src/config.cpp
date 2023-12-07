#include "config.hpp"
#include <iostream>

/**
 * @file config.cpp
 * @brief Содержит определение классов Config и ThreadSafeConfig,
 * которые реализуют интерфейс IThreadPool
 */


using namespace utility;

Config::Config(const std::filesystem::path &path, std::shared_ptr<spdlog::logger> logger) : IConfig(path, logger),
    logger_(logger) {
    parser = std::make_shared<JsonParser>(logger);
    try {
        this->path_ = makeNormalPath(path);
        parser->parse(path_);
        data_ = parser->outputConfig();
        notToUpdate = false;
    } catch(const std::exception &e) {
        if(logger_)
            logger_->error("There was thrown an exception while constructing config: {}", e.what());
        data_["RMin"] = 10;
        data_["RMax"] = 15;
        data_["AmountOfOperators"] = 2;
        data_["SizeOfQueue"] = 15;
        notToUpdate = true;
    }
}

std::pair<int, int> Config::getMinMax() {
    return std::make_pair(data_["RMin"], data_["RMax"]);
}

int Config::getAmountOfOperators() {
    return data_["AmountOfOperators"];
}

int Config::getSizeOfQueue() {
    return data_["SizeOfQueue"];
}

std::filesystem::path Config::getPath() {
    return path_;
}

bool Config::isUpdated() {
    return false;
}


void Config::updateConfig() {
    parser->parse(path_);
    data_ = parser->outputConfig();
    normalizeData();
}

void Config::updateWithRequest() {
    if(notToUpdate) {
        if (logger_)
            logger_->info("Could not run update thread because set flag not to update");
        return;
    }
    updateConfig();
    notify();
}

void Config::notify() {
    manager->update();
}

void Config::setManager(std::shared_ptr<IManager> manager) {
    this->manager = manager;
}

std::filesystem::path Config::makeNormalPath(const std::filesystem::path& pathToFile) {
    if(pathToFile.string().find('/') != std::string::npos) {
        return pathToFile;
    } else {
        auto currentPath = std::filesystem::current_path();
        return currentPath/pathToFile; 
    }
}

void Config::setLogger(std::shared_ptr<spdlog::logger> logger) {
    this->logger_ = logger;
}

void Config::normalizeData() {
    if(logger_)
        logger_->info("Normalizing data");

    normalizeRMinRMax();
    normalizeAmountOfOperators();
    normalizeSizeOfQueue();
}

void Config::normalizeRMinRMax() {
    if(logger_)
        logger_->info("Normalizing RMin RMax");

    if(data_["RMin"] >= 100)
        data_["RMin"] = 100;
    if(data_["RMax"] >= 140)
        data_["RMax"] = 140;
    if(data_["RMin"] <= 4)
        data_["RMin"] = 4;
    if(data_["RMax"] <= 5)
        data_["RMax"] = 5;
    if(data_["RMin"] > data_["RMax"])
        std::swap(data_["RMin"], data_["RMax"]);

    if(logger_) {
        logger_->debug("RMin: {} RMax: {} after normalizing", data_["RMin"], data_["RMax"]);
    }
}


void Config::normalizeAmountOfOperators() {
    if(logger_)
        logger_->info("Normalizing AmountOfOperators");
    if(data_["AmountOfOperators"] >= 80)
        data_["AmountOfOperators"] = 80;
    if(data_["AmountOfOperators"] <= 2)
        data_["AmountOfOperators"] = 2;

    if(logger_) {
        logger_->debug("AmountOfOperators: {} after normalizing", data_["AmountOfOperators"]);
    }
}


void Config::normalizeSizeOfQueue() {
    if(logger_)
        logger_->info("Normalizing SizeOfQueue");

    if(data_["SizeOfQueue"] >= 350)
        data_["SizeOfQueue"] = 350;
    if(data_["SizeOfQueue"] <= 15)
        data_["SizeOfQueue"] = 15;

    if(logger_) {
        logger_->debug("SizeOfQueue: {} after normalizing", data_["SizeOfQueue"]);
    }
}

ThreadSafeConfig::ThreadSafeConfig(const std::filesystem::path &path, std::shared_ptr<spdlog::logger> logger) :
    IConfig(path, logger), logger_(logger)  {
    parser = std::make_shared<JsonParser>(logger);
    try {
        this->path_ = makeNormalPath(path);
        parser->parse(path_);
        data_ = parser->outputConfig();
        notToUpdate = false;
    } catch(const std::exception &e) {
        if(logger_)
            logger_->error("There was thrown an exception while constructing config: {}", e.what());
        data_["RMin"] = 10;
        data_["RMax"] = 15;
        data_["AmountOfOperators"] = 2;
        data_["SizeOfQueue"] = 15;
        notToUpdate = true;
    }
    stopThread = false;
    updated = false;
}

ThreadSafeConfig::~ThreadSafeConfig() {
    stopThread = true;
    if (updateThread.joinable()) {
        updateThread.join();
    }
}

std::pair<int, int> ThreadSafeConfig::getMinMax() {
    std::lock_guard<std::mutex> lock(configMutex);
    if(logger_)
        logger_->debug("Getting MinMax values (RMin: {}, RMax: {})", data_["RMin"], data_["RMax"]);
    return std::make_pair(data_["RMin"], data_["RMax"]);
}

int ThreadSafeConfig::getAmountOfOperators() {
    std::lock_guard<std::mutex> lock(configMutex);
    if(logger_)
        logger_->debug("Getting AmountOfOperators (AmountOfOperators: {})", data_["AmountOfOperators"]);
    return data_["AmountOfOperators"];
}

int ThreadSafeConfig::getSizeOfQueue() {
    std::lock_guard<std::mutex> lock(configMutex);
    if(logger_)
        logger_->debug("Getting SizeOfQueue (SizeOfQueue: {}) ", data_["SizeOfQueue"]);
    return data_["SizeOfQueue"];
}

std::filesystem::path ThreadSafeConfig::getPath() {
    std::lock_guard<std::mutex> lock(configMutex);
    if(logger_)
        logger_->debug("Getting Path (Path: {})", path_.string());
    return path_;
}

void ThreadSafeConfig::updateConfig() {
    try {
        parser->parse(path_);
        if(logger_) {
            logger_->info("Configuration updated from file: {}", path_.string());
            logger_->info("New configuration {}", parser->output());
        }
        data_ = parser->outputConfig();
        normalizeData();
        updated = true;
    } catch (const std::exception& e) {
        if(logger_)
            logger_->error("Error updating configuration: {}", e.what());
        else
            throw e;
    }
}

bool ThreadSafeConfig::isUpdated() {
    std::lock_guard<std::mutex> lock(configMutex);
    bool result = updated;
    updated = false;
    if(logger_)
        logger_->debug("Checking if the configuration is updated: {}", result);
    return result;
}

void ThreadSafeConfig::notify() {
    if(manager) {
        if(logger_)
            logger_->info("Notifying manager about configuration update");
        manager->update();
    } else {
        if(logger_)
            logger_->warn("Manager not set. Unable to notify about configuration update");
    }
}

void ThreadSafeConfig::setManager(std::shared_ptr<IManager> manager) {
    this->manager = manager;
}

std::filesystem::path ThreadSafeConfig::makeNormalPath(const std::filesystem::path& pathToFile) {
    if(pathToFile.string().find('/') != std::string::npos) {
        return pathToFile;
    } else {
        auto currentPath = std::filesystem::current_path();
        return currentPath/pathToFile;
    }
}

std::time_t lastTime(const std::filesystem::path &filePath) {
    const auto fileTime = std::filesystem::last_write_time(filePath);
    const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
    const auto time = std::chrono::system_clock::to_time_t(systemTime);
    return time;
}

void ThreadSafeConfig::updateConfigThread() {
    try {

        lastWriteTime = lastTime(path_);
        while (!stopThread) {
            std::this_thread::sleep_for(std::chrono::seconds(60));
            std::time_t currentWriteTime = lastTime(path_);

            if (currentWriteTime > lastWriteTime) {
                if(logger_)
                    logger_->debug("Updating configuration from with regular checking");
                {
                    std::lock_guard<std::mutex> lock(configMutex);
                    updateConfig();
                }
                notify();
                lastWriteTime = currentWriteTime;
            }
        }
    } catch (std::exception& e) {
        if(logger_)
            logger_->critical("Critical error in updateConfigThread: {}", e.what());
        else
            throw e;
    }
}

void ThreadSafeConfig::updateWithRequest() {
    if(notToUpdate) {
        if (logger_)
            logger_->info("Could not run update thread because set flag not to update");
        return;
    }
    if(logger_)
        logger_->debug("Updating configuration from with request");
    {
        std::lock_guard<std::mutex> lock(configMutex);
        updateConfig();
    }
    notify();
    lastWriteTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void ThreadSafeConfig::RunMonitoring() {
    if(logger_)
        logger_->info("Starting configuration update monitoring thread");
    if(notToUpdate) {
        if(logger_)
            logger_->info("Could not run update thread because set flag not to update");
    } else {
        updateThread = std::thread(&ThreadSafeConfig::updateConfigThread, this);
    }
}

bool ThreadSafeConfig::isMonitoring() const {
    if(logger_)
        logger_->debug("Checking if monitor thread is running: {}", updateThread.joinable());
    return updateThread.joinable();
}

void ThreadSafeConfig::setLogger(std::shared_ptr<spdlog::logger> logger) {
    this->logger_ = logger;
    logger_->info("Logger set for ThreadSafeConfig");
}

void ThreadSafeConfig::normalizeData() {
    if(logger_)
        logger_->info("Normalizing data");

    normalizeRMinRMax();
    normalizeAmountOfOperators();
    normalizeSizeOfQueue();
}

void ThreadSafeConfig::normalizeRMinRMax() {
    if(logger_)
        logger_->info("Normalizing RMin RMax");

    if(data_["RMin"] >= 100)
        data_["RMin"] = 100;
    if(data_["RMax"] >= 140)
        data_["RMax"] = 140;
    if(data_["RMin"] <= 4)
        data_["RMin"] = 4;
    if(data_["RMax"] <= 5)
        data_["RMax"] = 5;
    if(data_["RMin"] > data_["RMax"])
        std::swap(data_["RMin"], data_["RMax"]);

    if(logger_) {
        logger_->debug("RMin: {} RMax: {} after normalizing", data_["RMin"], data_["RMax"]);
    }
}


void ThreadSafeConfig::normalizeAmountOfOperators() {
    if(logger_)
        logger_->info("Normalizing AmountOfOperators");
    if(data_["AmountOfOperators"] >= 80)
        data_["AmountOfOperators"] = 80;
    if(data_["AmountOfOperators"] <= 2)
        data_["AmountOfOperators"] = 2;

    if(logger_) {
        logger_->debug("AmountOfOperators: {} after normalizing", data_["AmountOfOperators"]);
    }
}


void ThreadSafeConfig::normalizeSizeOfQueue() {
    if(logger_)
        logger_->info("Normalizing SizeOfQueue");

    if(data_["SizeOfQueue"] >= 350)
        data_["SizeOfQueue"] = 350;
    if(data_["SizeOfQueue"] <= 15)
        data_["SizeOfQueue"] = 15;

    if(logger_) {
        logger_->debug("SizeOfQueue: {} after normalizing", data_["SizeOfQueue"]);
    }
}