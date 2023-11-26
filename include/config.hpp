
#ifndef PROTEI_COV_CONFIG_HPP
#define PROTEI_COV_CONFIG_HPP
#include <filesystem>
#include <thread>
#include <chrono>

#include "interfaces.hpp"

namespace utility {

class Config : public utility::IConfig {
public:
    explicit Config(const std::filesystem::path &path);

    std::pair<int, int> getMinMax();

    int getAmountOfOperators();

    int getSizeOfQueue();
    
    std::filesystem::path getPath();

    void updateConfig();

    bool isUpdated();

    void notify();

    void setManager(std::shared_ptr<IManager> manager);
private:
    static std::filesystem::path makeNormalPath(const std::filesystem::path& pathToFile);

    JsonParser parser;

    std::filesystem::path path_;

    std::map<std::string, int> data_;

    std::shared_ptr<IManager> manager;
};

class ThreadSafeConfig : public utility::IConfig {
public:
    explicit ThreadSafeConfig(const std::filesystem::path &path);

    ~ThreadSafeConfig();

    std::pair<int, int> getMinMax();

    int getAmountOfOperators();

    int getSizeOfQueue();

    std::filesystem::path getPath();

    void updateConfig();

    bool isUpdated();

    void updateConfigThread();

    void notify();

    void setManager(std::shared_ptr<IManager> manager);
private:
    static std::filesystem::path makeNormalPath(const std::filesystem::path& pathToFile);

    JsonParser parser;

    std::filesystem::path path_;

    std::map<std::string, int> data_;

    std::thread updateThread; // Поток для обновления конфигурации
    std::mutex configMutex; // Мьютекс для защиты доступа к конфигурации
    bool stopThread; // Флаг для остановки потока
    bool updated; // Флаг, указывающий на обновление конфигурации
    std::time_t lastWriteTime;
    std::shared_ptr<IManager> manager;
};
}
#endif