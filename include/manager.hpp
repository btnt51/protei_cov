//
// Created by Евгений Байлов on 19.11.2023.
//

#ifndef PROTEI_COV_MANAGER_HPP
#define PROTEI_COV_MANAGER_HPP
#include <filesystem>
#include <future>

#include "commonStructures.hpp"
#include "jsonParser.hpp"
#include "threadpool.hpp"
/**
 * @class Manager
 * @brief Класс управляющей тредпулом (центром обратки вызовов)
 */
class Manager {
public:
    Manager(std::string_view pathToConfig);

    void startMonitoringConfigFile() {
        stopMonitoring = false;
        monitorThread = std::thread([this] { monitorConfigFile(); });
    }

    void stopMonitoringConfigFile() {
        stopMonitoring = true;
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
    }

    std::future<Result> addTask(std::string_view number);
private:
    void monitorConfigFile();

    const std::filesystem::path& pathToConfigFile;
    TP::ThreadPool threadPool;
    std::atomic<bool> stopMonitoring;
    std::thread monitorThread;
};

#endif // PROTEI_COV_MANAGER_HPP
