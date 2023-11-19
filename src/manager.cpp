
#include "manager.hpp"
#include <iostream>

Manager::Manager(const std::string& pathToConfig): pathToConfigFile(std::filesystem::path(pathToConfig)) {
    parser.parse(pathToConfigFile);
    auto json = parser.outputConfig();
    RMin_ = json["RMin"];
    RMax_ = json["RMax"];
    threadPool = std::make_shared<TP::ThreadPool>(json["AmountOfOperators"]);
    threadPool->start();
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

void Manager::monitorConfigFile() { }

void Manager::updateThreadPool() { }