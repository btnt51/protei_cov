#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "manager.hpp"

#include "interfaces.hpp"
#include "threadpool.hpp"

class MockQueue : public TP::IQueue {
public:
    MockQueue(int size) : TP::IQueue(size) { }
    MOCK_METHOD((std::pair<std::shared_ptr<TP::ITask>, TP::CallID>&), back, (),(override));
    MOCK_METHOD((std::pair<std::shared_ptr<TP::ITask>, TP::CallID>&), front, (),(override));
    MOCK_METHOD(bool, empty, (), (const, override));
    MOCK_METHOD(bool, push, ((std::pair<std::shared_ptr<TP::ITask>, TP::CallID>&& taskPair)), (override));
    MOCK_METHOD(void, pop, (), (override));
    MOCK_METHOD(void, update, (int size), (override));
    MOCK_METHOD(void, setLogger, ((std::shared_ptr<spdlog::logger>)), (override));
    MOCK_METHOD(void, setRecorders, ((std::vector<std::shared_ptr<IRecorder>> recorders)), (override));
    MOCK_METHOD(void, writeCDR, (const CDR& cdr), (override));
};

class MockConfig : public utility::IConfig {
public:
    MockConfig(const std::filesystem::path& path, std::shared_ptr<spdlog::logger> logger) : IConfig(path, logger) { }
    MOCK_METHOD((std::pair<int, int>), getMinMax, (), (override));
    MOCK_METHOD(int, getAmountOfOperators, (), (override));
    MOCK_METHOD(int, getSizeOfQueue, (), (override));
    MOCK_METHOD(std::filesystem::path, getPath, (), (override));
    MOCK_METHOD(void, updateConfig, (), (override));
    MOCK_METHOD(bool, isUpdated, (), (override));
    MOCK_METHOD(void, notify, (), (override));
    MOCK_METHOD(void, setManager, (std::shared_ptr<IManager> manager), (override));
    MOCK_METHOD(void, setLogger, ((std::shared_ptr<spdlog::logger>)), (override));
    MOCK_METHOD(void, updateWithRequest, (), (override));
};

// Mock для IThreadPool
class MockThreadPool : public TP::IThreadPool {
public:
    MockThreadPool(unsigned amountOfThreads, unsigned sizeOfQueue) : TP::IThreadPool(amountOfThreads, sizeOfQueue) {}

    MOCK_METHOD((std::pair<TP::CallID, std::future<Result>>), add_task, (std::shared_ptr<TP::ITask> task), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, transferObjects, (const std::shared_ptr<TP::IThreadPool>& oldThreadPool), (override));
    MOCK_METHOD(void, writeCDR, (CDR& cdr), (override));
    MOCK_METHOD(void, setTaskQueue, (std::shared_ptr<TP::IQueue> task_queue), (override));
    MOCK_METHOD(void, setLogger, ((std::shared_ptr<spdlog::logger>)), (override));
};

class ManagerTest : public testing::Test {
protected:
    void SetUp() override {
        mockConfig = std::make_shared<MockConfig>("base.json", nullptr);
        mockThreadPool = std::make_shared<MockThreadPool>(2, 4);
        mockQueue = std::make_shared<MockQueue>(4);
        mockThreadPool->setTaskQueue(mockQueue);
        manager = std::make_shared<Manager>(mockConfig, mockThreadPool);
        //mockConfig->setManager(manager);
    }

    void TearDown() override {
        // Очистка объектов после каждого теста
    }

    std::shared_ptr<Manager> manager;
    std::shared_ptr<MockThreadPool> mockThreadPool;
    std::shared_ptr<MockConfig> mockConfig;
    std::shared_ptr<MockQueue> mockQueue;
};


//Тест конструктора Manager
TEST_F(ManagerTest, ConstructorTest) {
    EXPECT_CALL(*mockConfig, getMinMax()).WillOnce(::testing::Return(std::make_pair(10,20)));
    auto mockThreadPool1 = std::make_shared<MockThreadPool>(2, 4);
    auto manager1 = std::make_shared<Manager>(mockConfig, mockThreadPool1);

    std::cout << "Checking if manager is not nullptr" << std::endl;
    ASSERT_NE(manager1, nullptr);
}

TEST_F(ManagerTest, SetThreadPool) {
    EXPECT_CALL(*mockThreadPool, transferObjects(::testing::_)).Times(1);
    EXPECT_CALL(*mockThreadPool, start()).Times(1);
    EXPECT_CALL(*mockThreadPool, stop()).Times(1);
    manager->setNewThreadPool(mockThreadPool);
}

TEST_F(ManagerTest, threadPoolStartTest) {
    EXPECT_CALL(*mockThreadPool, start()).Times(1);
    manager->startThreadPool();
}

TEST_F(ManagerTest, threadPoolStopTest) {
    EXPECT_CALL(*mockThreadPool, stop()).Times(1);
    manager->stopThreadPool();
}

TEST_F(ManagerTest, AddTaskTest) {
    EXPECT_CALL(*mockThreadPool, add_task(::testing::_)).WillOnce(::testing::Return(std::make_pair(10, std::future<Result>())));
    manager->addTask("test_num");
}

TEST_F(ManagerTest, UpdateFunction) {
    EXPECT_CALL(*mockConfig, getMinMax()).WillOnce(::testing::Return(std::make_pair(10,20)));
    EXPECT_CALL(*mockConfig, getAmountOfOperators()).WillOnce(::testing::Return(2));
    EXPECT_CALL(*mockConfig, getSizeOfQueue()).Times(2).WillRepeatedly(::testing::Return(4));
    EXPECT_CALL(*mockQueue, empty()).Times(::testing::AnyNumber()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*mockQueue, update(::testing::_)).Times(1);
    auto threadPool = std::make_shared<TP::ThreadPool>(2, 4);
    manager->setNewThreadPool(threadPool);
    threadPool->setTaskQueue(mockQueue);
    manager->update();
}

TEST_F(ManagerTest, updateViaRequest) {
    EXPECT_CALL(*mockConfig, updateWithRequest()).Times(1);
    EXPECT_CALL(*mockConfig, isUpdated()).Times(1).WillOnce(::testing::Return(true));
    auto res = manager->processRequestForUpdate();
    ASSERT_TRUE(res);
}



