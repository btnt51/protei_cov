#include "builder.hpp"
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ansicolor_sink.h>

/**
 * @file builder.hpp
 * @brief Содержит определение класса ManagerBuilder,
 * частично реализующий паттерн Строитель.
 */

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
    try {
        config = std::make_shared<utility::ThreadSafeConfig>(pathToConfig, logger);
        logger->info("Built config");
        return config;
    } catch (std::exception& e) {
        logger->critical("Critical error building config: {}", e.what());
        throw e;
    }
}

std::vector<std::shared_ptr<IRecorder>> ManagerBuilder::BuildRecorders() {
    try {
        if(recorders.empty()) {
            logger->info("Built recorders");
            recorders = std::vector<std::shared_ptr<IRecorder>>{};
            auto fileRecorder = std::make_shared<FileRecorder>("cdrFile.txt");
            recorders.push_back(fileRecorder);

            for(const auto& recorder: recorders)
                recorder->setLogger(logger);
        }
        return recorders;
    }  catch (std::exception& e) {
        logger->critical("Critical error building config: {}", e.what());
        throw e;
    }
}

std::shared_ptr<TP::ThreadPool> ManagerBuilder::BuildThreadPool() {
    try {
        auto pool = std::make_shared<TP::ThreadPool>(config->getAmountOfOperators(), config->getSizeOfQueue());
        logger->info("Built thread pool");
        pool->setLogger(logger);
        return pool;
    } catch (std::exception& e) {
        logger->critical("Critical error building thread pool: {}", e.what());
        throw e;
    }
}

std::shared_ptr<Manager> ManagerBuilder::Construct(const std::filesystem::path& pathToConfig) {
    try {
        logger = BuildLogger();
        config = BuildConfig(pathToConfig);
        BuildRecorders();
        auto pool = BuildThreadPool();

        auto manager = std::make_shared<Manager>(config, pool);

        manager->setLogger(logger);
        config->RunMonitoring();
        config->setManager(manager);

        logger->info("Manager constructed successfully");

        return manager;
    } catch (std::exception& e) {
        logger->critical("Critical error constructing manager: {}", e.what());

        throw e;
    }
}