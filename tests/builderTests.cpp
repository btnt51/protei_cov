#include "builder.hpp"
#include <gtest/gtest.h>
#include <spdlog/async.h>

class ManagerBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
        managerBuilder = std::make_shared<ManagerBuilder>();
    }

    void TearDown() override {

    }

    std::shared_ptr<ManagerBuilder> managerBuilder;
};

TEST_F(ManagerBuilderTest, BuildLoggerTest) {
    spdlog::init_thread_pool(8192, 1);
    ASSERT_NE(managerBuilder->BuildLogger(), nullptr);
}

TEST_F(ManagerBuilderTest, BuildConfigTest) {
    spdlog::init_thread_pool(8192, 1);
    managerBuilder->BuildLogger();
    ASSERT_NE(managerBuilder->BuildConfig("base.json"), nullptr);
}

TEST_F(ManagerBuilderTest, BuildThreadPoolTest) {
    spdlog::init_thread_pool(8192, 1);
    managerBuilder->BuildLogger();
    managerBuilder->BuildConfig("base.json");
    ASSERT_NE(managerBuilder->BuildThreadPool(), nullptr);
}

TEST_F(ManagerBuilderTest, ConstructTest) {
    spdlog::init_thread_pool(8192, 1);
    ASSERT_NE(managerBuilder->Construct("base.json"), nullptr);
}