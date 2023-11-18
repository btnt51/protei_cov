//
// Created by Евгений Байлов on 15.11.2023.
//
#include <gtest/gtest.h>
#include <string>

TEST(Example_test, example){
    std::string test = "test";
    ASSERT_EQ(test, "test");
}