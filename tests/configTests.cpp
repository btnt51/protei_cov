#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "interfaces.hpp"
#include "config.hpp"
#include "threadpool.hpp"

class MockManager : public IManager {
public:
    MockManager(std::shared_ptr<utility::IConfig> conf, std::shared_ptr<TP::IThreadPool> pool)
        : IManager(conf, pool) {}

    MOCK_METHOD((std::pair<TP::CallID, std::future<Result>>), addTask, (std::string_view number), (override));
    MOCK_METHOD(void, startThreadPool, (), (override));
    MOCK_METHOD(void, stopThreadPool, (), (override));
    MOCK_METHOD(void, setNewConfig,(std::shared_ptr<utility::IConfig> config), (override));
    MOCK_METHOD(void, setNewThreadPool, (std::shared_ptr<TP::IThreadPool> pool), (override));
    MOCK_METHOD(void, setLogger, ((std::shared_ptr<spdlog::logger> logger)), (override));
    MOCK_METHOD(void, update,(), (override));
    MOCK_METHOD(bool, processRequestForUpdate, (), (override));
};


class MockThreadSafeConfig : public utility::ThreadSafeConfig {
public:
    explicit MockThreadSafeConfig(const std::filesystem::path& path, std::shared_ptr<spdlog::logger> logger)
        : ThreadSafeConfig(path, logger) {}

    MOCK_METHOD(void, updateConfigThread, ());
};

// Ваши тесты для ThreadSafeConfig
class ThreadSafeConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_shared<utility::ThreadSafeConfig>("base.json", nullptr);
        threadPool = std::make_shared<TP::ThreadPool>(config->getAmountOfOperators(), config->getSizeOfQueue());
        mockManager = std::make_shared<MockManager>(config, threadPool);
        config->setManager(mockManager);
    }

    void TearDown() override {
    }

    std::shared_ptr<MockManager> mockManager;
    std::shared_ptr<utility::ThreadSafeConfig> config;
    std::shared_ptr<TP::IThreadPool> threadPool;
};

TEST_F(ThreadSafeConfigTest, GetMinMax) {
    auto result = config->getMinMax();
    int expectedMinValue = 10;
    int expectedMaxValue = 15;
    ASSERT_EQ(result.first, expectedMinValue);
    ASSERT_EQ(result.second, expectedMaxValue);
}

TEST_F(ThreadSafeConfigTest, GetAmountOfOperators) {
    auto result = config->getAmountOfOperators();
    int expectedOperators = 5;
    ASSERT_EQ(result, expectedOperators);
}

TEST_F(ThreadSafeConfigTest, GetSizeOfQueue) {
    auto result = config->getSizeOfQueue();
    int expectedSizeOfQueue = 15;
    ASSERT_EQ(result, expectedSizeOfQueue);
}

TEST_F(ThreadSafeConfigTest, Notify) {
    EXPECT_CALL(*mockManager, update()).Times(1);
    config->notify();
}

TEST_F(ThreadSafeConfigTest, GetPath) {
    auto result = config->getPath();
    std::filesystem::path expectedPath = std::filesystem::current_path()/"base.json";
    ASSERT_EQ(result, expectedPath);
}

TEST_F(ThreadSafeConfigTest, GetMinMaxUpdate) {
    config->updateConfig();
    auto result = config->getMinMax();
    int expectedMinValue = 10;
    int expectedMaxValue = 15;
    ASSERT_EQ(result.first, expectedMinValue);
    ASSERT_EQ(result.second, expectedMaxValue);
}

TEST_F(ThreadSafeConfigTest, GetAmountOfOperatorsUpdate) {
    config->updateConfig();
    auto result = config->getAmountOfOperators();
    int expectedOperators = 5;
    ASSERT_EQ(result, expectedOperators);
}

TEST_F(ThreadSafeConfigTest, GetSizeOfQueueUpdate) {
    config->updateConfig();
    auto result = config->getSizeOfQueue();
    int expectedOperators = 15;
    ASSERT_EQ(result, expectedOperators);
}

TEST_F(ThreadSafeConfigTest, IsUpdatedWithoutUpdate) {
    auto result = config->isUpdated();
    ASSERT_TRUE(!result);
}

TEST_F(ThreadSafeConfigTest, IsUpdatedWithUpdate) {
    config->updateConfig();
    auto result = config->isUpdated();
    ASSERT_TRUE(result);
}

TEST_F(ThreadSafeConfigTest, GetMinMaxUpdateTestUpperBorder) {
    config = std::make_shared<utility::ThreadSafeConfig>("upperBorder.json", nullptr);
    config->updateConfig();
    auto result = config->getMinMax();
    int expectedMinValue = 100;
    int expectedMaxValue = 140;
    ASSERT_EQ(result.first, expectedMinValue);
    ASSERT_EQ(result.second, expectedMaxValue);
}

TEST_F(ThreadSafeConfigTest, GetAmountOfOperatorsUpdateTestUpperBorder) {
    config = std::make_shared<utility::ThreadSafeConfig>("upperBorder.json", nullptr);
    config->updateConfig();
    auto result = config->getAmountOfOperators();
    int expectedOperators = 80;
    ASSERT_EQ(result, expectedOperators);
}

TEST_F(ThreadSafeConfigTest, GetSizeOfQueueUpdateTestUpperBorder) {
    config = std::make_shared<utility::ThreadSafeConfig>("upperBorder.json", nullptr);
    config->updateConfig();
    auto result = config->getSizeOfQueue();
    int expectedSizeOfQueue = 350;
    ASSERT_EQ(result, expectedSizeOfQueue);
}

TEST_F(ThreadSafeConfigTest, GetMinMaxUpdateTestLowereBorder) {
    config = std::make_shared<utility::ThreadSafeConfig>("lowerBorder.json", nullptr);
    config->updateConfig();
    auto result = config->getMinMax();
    int expectedMinValue = 4;
    int expectedMaxValue = 5;
    ASSERT_EQ(result.first, expectedMinValue);
    ASSERT_EQ(result.second, expectedMaxValue);
}

TEST_F(ThreadSafeConfigTest, GetAmountOfOperatorsUpdateTestLowereBorder) {
    config = std::make_shared<utility::ThreadSafeConfig>("lowerBorder.json", nullptr);
    config->updateConfig();
    auto result = config->getAmountOfOperators();
    int expectedOperators = 2;
    ASSERT_EQ(result, expectedOperators);
}

TEST_F(ThreadSafeConfigTest, GetSizeOfQueueUpdateTestLowereBorder) {
    config = std::make_shared<utility::ThreadSafeConfig>("lowerBorder.json", nullptr);
    config->updateConfig();
    auto result = config->getSizeOfQueue();
    int expectedSizeOfQueue = 15;
    ASSERT_EQ(result, expectedSizeOfQueue);
}

TEST_F(ThreadSafeConfigTest, DefaultDataWithBrokenFile) {
    auto configUpdateThread = std::make_shared<MockThreadSafeConfig>("broken.json", nullptr);
    auto RMinRMax = config->getMinMax();
    auto AmountOfOperators = config->getAmountOfOperators();
    auto SizeOfQueue = config->getSizeOfQueue();
    int expectedMinValue = 10;
    int expectedMaxValue = 15;
    int expectedSizeOfQueue = 15;
    int expectedOperators = 5;
    ASSERT_EQ(RMinRMax.first, expectedMinValue);
    ASSERT_EQ(RMinRMax.second, expectedMaxValue);
    ASSERT_EQ(AmountOfOperators, expectedOperators);
    ASSERT_EQ(SizeOfQueue, expectedSizeOfQueue);
}

TEST_F(ThreadSafeConfigTest, ProcessRequestToUpdate) {
    EXPECT_CALL(*mockManager, update()).Times(1);
    config->updateWithRequest();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(config->isUpdated());
}

TEST_F(ThreadSafeConfigTest, RunMonitoringStartsThreadAfterBrokenFile) {
    auto configUpdateThread = std::make_shared<MockThreadSafeConfig>("broken.json", nullptr);
    configUpdateThread->setManager(mockManager);
    configUpdateThread->RunMonitoring();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_FALSE(configUpdateThread->isMonitoring());
}

TEST_F(ThreadSafeConfigTest, RunMonitoringStartsThread) {
    auto configUpdateThread = std::make_shared<MockThreadSafeConfig>("base.json", nullptr);
    configUpdateThread->setManager(mockManager);
    configUpdateThread->RunMonitoring();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(configUpdateThread->isMonitoring());
}
