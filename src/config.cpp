#include "config.hpp"
#include <iostream>

using namespace utility;

Config::Config(const std::filesystem::path &path) : IConfig(path) {
    this->path_ = makeNormalPath(path);
    parser.parse(path_);
    data_ = parser.outputConfig();
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
    parser.parse(path_);
    data_ = parser.outputConfig();
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


ThreadSafeConfig::ThreadSafeConfig(const std::filesystem::path &path) :
    IConfig(path) {
    this->path_ = makeNormalPath(path);
    std::cout << "path:" << path_ << std::endl;
    parser.parse(path_);
    std::cout << parser.output() << std::endl;
    data_ = parser.outputConfig();
    stopThread = false;
    updateThread = std::thread(&ThreadSafeConfig::updateConfigThread, this);
}

ThreadSafeConfig::~ThreadSafeConfig() {
    stopThread = true;
    if (updateThread.joinable()) {
        updateThread.join();
    }
}

std::pair<int, int> ThreadSafeConfig::getMinMax() {
    std::lock_guard<std::mutex> lock(configMutex);
    return std::make_pair(data_["RMin"], data_["RMax"]);
}

int ThreadSafeConfig::getAmountOfOperators() {
    std::lock_guard<std::mutex> lock(configMutex);
    return data_["AmountOfOperators"];
}

int ThreadSafeConfig::getSizeOfQueue() {
    std::lock_guard<std::mutex> lock(configMutex);
    return data_["SizeOfQueue"];
}

std::filesystem::path ThreadSafeConfig::getPath() {
    std::lock_guard<std::mutex> lock(configMutex);
    return path_;
}

void ThreadSafeConfig::updateConfig() {
    parser.parse(path_);
    std::cout << parser.output() << std::endl;
    data_ = parser.outputConfig();
    updated = true;
}

bool ThreadSafeConfig::isUpdated() {
    std::lock_guard<std::mutex> lock(configMutex);
    bool result = updated;
    updated = false;
    return result;
}

void ThreadSafeConfig::notify() {
    if(manager) {
        std::cout << "I was here and notified manager" << std::endl;
        manager->update();
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
            // Периодически обновляем конфигурацию (например, каждые 5 секунд)
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::time_t currentWriteTime = lastTime(path_);

            if (currentWriteTime != lastWriteTime) {
                {
                    std::lock_guard<std::mutex> lock(configMutex);
                    updateConfig();
                }
                notify();
                lastWriteTime = currentWriteTime;
            }
            // Защищаем доступ к конфигурации мьютексом

            // Вызываем функцию обновления конфигурации
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}