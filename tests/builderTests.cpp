#include <gtest/gtest.h>
#include "builder.hpp"

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
    ASSERT_NE(managerBuilder->BuildLogger(), nullptr);

}

TEST_F(ManagerBuilderTest, BuildConfigTest) {
    // Аналогично, напишите тест для метода BuildConfig
    managerBuilder->BuildLogger();
    ASSERT_NE(managerBuilder->BuildConfig("base.json"), nullptr);
}

TEST_F(ManagerBuilderTest, BuildThreadPoolTest) {
    managerBuilder->BuildLogger();
    managerBuilder->BuildConfig("base.json");
    ASSERT_NE(managerBuilder->BuildThreadPool(), nullptr);
}

TEST_F(ManagerBuilderTest, ConstructTest) {
    ASSERT_NE(managerBuilder->Construct("base.json"), nullptr);
}