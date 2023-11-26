#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "manager.hpp"

#include "interfaces.hpp"

class MockConfig : public utility::IConfig {
public:
    MockConfig(const std::filesystem::path& path) : IConfig(path) { }
    MOCK_METHOD((std::pair<int, int>), getMinMax, (), (override));
    MOCK_METHOD(int, getAmountOfOperators, (), (override));
    MOCK_METHOD(int, getSizeOfQueue, (), (override));
    MOCK_METHOD(std::filesystem::path, getPath, (), (override));
    MOCK_METHOD(void, updateConfig, (), (override));
    MOCK_METHOD(bool, isUpdated, (), (override));
    MOCK_METHOD(void, notify, (), (override));
    MOCK_METHOD(void, setManager, (std::shared_ptr<IManager> manager), (override));
};

// Mock для IThreadPool
class MockThreadPool : public TP::IThreadPool {
public:
    MockThreadPool(unsigned amountOfThreads) : TP::IThreadPool(amountOfThreads) {}

    MOCK_METHOD((std::pair<TP::CallID, std::future<Result>>), add_task, (const TP::Task& task), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, transferTaskQueue, (const std::shared_ptr<TP::IThreadPool>& oldThreadPool), (override));
};

class ManagerTest : public testing::Test {
protected:
    void SetUp() override {
        mockConfig = std::make_shared<MockConfig>("base.json");
        mockThreadPool = std::make_shared<MockThreadPool>(2);
        manager = std::make_shared<Manager>(mockConfig, mockThreadPool);
        mockConfig->setManager(manager);
    }

    void TearDown() override {
        // Очистка объектов после каждого теста
    }

    std::shared_ptr<Manager> manager;
    std::shared_ptr<MockThreadPool> mockThreadPool;
    std::shared_ptr<MockConfig> mockConfig;
};


//Тест конструктора Manager
TEST_F(ManagerTest, ConstructorTest) {
    //EXPECT_CALL(*mockConfig, setManager(manager)).Times(1);
    EXPECT_CALL(*mockConfig, getMinMax()).WillOnce(::testing::Return(std::make_pair(10,20)));
    auto mockThreadPool1 = std::make_shared<MockThreadPool>(2);
    auto manager1 = std::make_shared<Manager>(mockConfig, mockThreadPool1);

    std::cout << "Checking if manager is not nullptr" << std::endl;
    ASSERT_NE(manager1, nullptr);
}

TEST_F(ManagerTest, SetThreadPool) {
    EXPECT_CALL(*mockThreadPool, start()).Times(1);
    EXPECT_CALL(*mockThreadPool, transferTaskQueue(::testing::_)).Times(1);
    EXPECT_CALL(*mockThreadPool, stop()).Times(1);
    manager->setNewThreadPool(mockThreadPool);
}

TEST_F(ManagerTest, UpdateFunction) {
    EXPECT_CALL(*mockConfig, getMinMax()).WillOnce(::testing::Return(std::make_pair(10,20)));
    EXPECT_CALL(*mockConfig, getAmountOfOperators()).WillOnce(::testing::Return(2));
    manager->update();
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

