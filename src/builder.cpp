#include "builder.hpp"
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ansicolor_sink.h>

std::shared_ptr<spdlog::logger> ManagerBuilder::BuildLogger() {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logfile.txt", true);

    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_logger",
        spdlog::sinks_init_list{file_sink, std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>()},
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    async_logger->set_level(spdlog::level::debug);

    async_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");


    logger = async_logger;
    return async_logger;
}

std::shared_ptr<utility::ThreadSafeConfig> ManagerBuilder::BuildConfig(const std::filesystem::path& pathToConfig) {
    config = std::make_shared<utility::ThreadSafeConfig>(pathToConfig);
    logger->info("Built config");
    config->setLogger(logger);


    return config;
}

std::shared_ptr<TP::ThreadPool> ManagerBuilder::BuildThreadPool() {
    auto pool = std::make_shared<TP::ThreadPool>(config->getAmountOfOperators(), config->getSizeOfQueue());
    pool->setLogger(logger);
    return pool;
}

std::shared_ptr<Manager> ManagerBuilder::Construct(const std::filesystem::path& pathToConfig) {
    logger = BuildLogger();
    config = BuildConfig(pathToConfig);
    auto pool = BuildThreadPool();

    auto manager = std::make_shared<Manager>(config, pool);

    manager->setLogger(logger);
    config->RunMonitoring();
    config->setManager(manager);

    return manager;
}