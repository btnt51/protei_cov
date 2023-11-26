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
    MOCK_METHOD(void, update,(), (override));
};

// Ваши тесты для ThreadSafeConfig
class ThreadSafeConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_shared<utility::ThreadSafeConfig>("base.json");
        threadPool = std::make_shared<TP::ThreadPool>(config->getAmountOfOperators());
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
    int expectedOperators = 13;
    ASSERT_EQ(result, expectedOperators);
}

TEST_F(ThreadSafeConfigTest, Notify) {
    EXPECT_CALL(*mockManager, update()).Times(1);
    config->notify();
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
    // Предположим, что у вас есть данные в конфигурации
    config->updateConfig();
    auto result = config->getSizeOfQueue();
    int expectedOperators = 13;
    ASSERT_EQ(result, expectedOperators);
}
