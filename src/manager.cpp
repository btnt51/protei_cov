
#include "manager.hpp"
#include <iostream>

Manager::Manager(const std::string& pathToConfig): pathToConfigFile(std::filesystem::path(pathToConfig)) {
    parser.parse(pathToConfigFile);
    lastFileModificationTime = getLastModificationTime();
    auto json = parser.outputConfig();
    RMin_ = json["RMin"];
    RMax_ = json["RMax"];
    threadPool = std::make_shared<TP::ThreadPool>(json["AmountOfOperators"]);
    threadPool->start();
    startMonitoringConfigFile();
}

void Manager::startMonitoringConfigFile() {
    stopMonitoring = false;
    monitorThread = std::thread([this] { monitorConfigFile(); });
}

void Manager::stopMonitoringConfigFile() {
    stopMonitoring = true;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

std::pair<TP::CallID, std::future<Result>> Manager::addTask(std::string_view number) {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // TODO: тут должно быть лог сообщение
    TP::Task task = TP::Task(RMin_, RMax_, number, now);
    task.addPromise(std::make_shared<std::promise<Result>>());
    return threadPool->add_task(task);
}

void Manager::monitorConfigFile() {
    while (!stopMonitoring) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto currentModificationTime = getLastModificationTime();
            if (currentModificationTime != lastFileModificationTime) {
                updateThreadPool();
                lastFileModificationTime = currentModificationTime;
        }
    }
}

std::time_t Manager::getLastModificationTime() {
    auto time = std::chrono::file_clock::to_sys(std::filesystem::last_write_time(pathToConfigFile));
    return std::chrono::system_clock::to_time_t(time);
}

void Manager::updateThreadPool() {
    parser.parse(pathToConfigFile);
    auto json = parser.outputConfig();
    RMin_ = json["RMin"];
    RMax_ = json["RMax"];
    threadPool->stop();
    auto newThreadPool = std::make_shared<TP::ThreadPool>(json["AmountOfOperators"]);
    newThreadPool->transferTaskQueue(threadPool);
    threadPool = std::move(newThreadPool);
    threadPool->start();
}