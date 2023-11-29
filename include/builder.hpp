#ifndef PROTEI_COV_BUILDER_HPP
#define PROTEI_COV_BUILDER_HPP
#include "manager.hpp"
#include "config.hpp"
#include "threadpool.hpp"

class ManagerBuilder {
public:
    std::shared_ptr<spdlog::logger> BuildLogger();

    std::shared_ptr<utility::ThreadSafeConfig> BuildConfig(const std::filesystem::path& pathToConfig);

    std::shared_ptr<TP::ThreadPool> BuildThreadPool();

    std::shared_ptr<Manager> Construct(const std::filesystem::path& pathToConfig);
private:
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<utility::ThreadSafeConfig> config;
};

#endif // PROTEI_COV_BUILDER_HPP
