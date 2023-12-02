#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "jsonParser.hpp"

class JsonParserTest : public ::testing::Test {
protected:
    void SetUp() override {

        jsonParser_ = std::make_unique<utility::JsonParser>(nullptr);
    }

    void TearDown() override {

    }

    std::unique_ptr<utility::JsonParser> jsonParser_;
};

TEST_F(JsonParserTest, ParseTest) {
    const std::filesystem::path testFilePath = "base.json";
    EXPECT_NO_THROW(jsonParser_->parse(testFilePath));
}

TEST_F(JsonParserTest, OutputTest) {
    jsonParser_->parse("base.json");
    EXPECT_EQ(jsonParser_->output(), "RMin: 10\nRMax: 15\nAmountOfOperators: 5\nSizeOfQueue: 7\n");
}

TEST_F(JsonParserTest, OutputConfigTest) {
    jsonParser_->parse("base.json");
    std::map<std::string, int> expectedConfig = {
        {"RMin", 10},
        {"RMax", 15},
        {"AmountOfOperators", 5},
        {"SizeOfQueue", 7}
    };
    EXPECT_EQ(jsonParser_->outputConfig(), expectedConfig);
}