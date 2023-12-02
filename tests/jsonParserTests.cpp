#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "jsonparser.hpp"  // Замените на правильный путь к вашему заголовочному файлу

class JsonParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Любые предварительные действия, которые нужны для ваших тестов
        jsonParser_ = std::make_unique<utility::JsonParser>(nullptr);
    }

    void TearDown() override {
        // Любые завершающие действия после ваших тестов
    }

    std::unique_ptr<utility::JsonParser> jsonParser_;
};

TEST_F(JsonParserTest, ParseTest) {
    const std::filesystem::path testFilePath = "base.json";
    EXPECT_NO_THROW(jsonParser_->parse(testFilePath));
}

TEST_F(JsonParserTest, OutputTest) {
    // Подготовьте тестовые данные
    jsonParser_->parse("base.json");  // Парсинг тестового файла
    // Утверждайте, что метод output возвращает ожидаемую строку
    EXPECT_EQ(jsonParser_->output(), "RMin: 10\nRMax: 15\nAmountOfOperators: 5\nSizeOfQueue: 7\n");
}

TEST_F(JsonParserTest, OutputConfigTest) {
    // Подготовьте тестовые данные
    jsonParser_->parse("base.json");  // Парсинг тестового файла
    // Утверждайте, что метод outputConfig возвращает ожидаемую карту
    std::map<std::string, int> expectedConfig = {
        {"RMin", 10},
        {"RMax", 15},
        {"AmountOfOperators", 5},
        {"SizeOfQueue", 7}
    };
    EXPECT_EQ(jsonParser_->outputConfig(), expectedConfig);
}