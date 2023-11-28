#include <gtest/gtest.h>
#include "threadpool.hpp"
#include "queue.hpp"

TEST(QueueTest, Push) {
    TP::Queue queue(3);
    auto time = std::time_t{2};
    auto task1 = std::make_shared<TP::Task>(1, 2, "1", time);
    auto task2 = std::make_shared<TP::Task>(1, 2, "2", time);
    auto task3 = std::make_shared<TP::Task>(1, 2, "3", time);

    EXPECT_TRUE(queue.push(std::make_pair(task1, 1)));
    EXPECT_TRUE(queue.push(std::make_pair(task2, 2)));
    EXPECT_TRUE(queue.push(std::make_pair(task3, 3)));
    EXPECT_FALSE(queue.push(std::make_pair(std::make_shared<TP::Task>(1,2,"4", time), 4)));
}

TEST(QueueTest, FrontBack) {
    TP::Queue queue(3);
    auto time = std::time_t{2};
    auto task1 = std::make_shared<TP::Task>(1, 2, "1", time);
    auto task2 = std::make_shared<TP::Task>(1, 2, "2", time);
    auto task3 = std::make_shared<TP::Task>(1, 2, "3", time);

    EXPECT_TRUE(queue.push(std::make_pair(task1, 1)));
    EXPECT_TRUE(queue.push(std::make_pair(task2, 2)));
    EXPECT_TRUE(queue.push(std::make_pair(task3, 3)));

    auto frontTask = queue.front();
    auto backTask = queue.back();
    EXPECT_EQ(frontTask.first, task1);
    EXPECT_EQ(backTask.first, task3);
}

TEST(QueueTest, Pop) {
    TP::Queue queue(3);
    auto time = std::time_t{2};
    auto task1 = std::make_shared<TP::Task>(1, 2, "1", time);
    auto task2 = std::make_shared<TP::Task>(1, 2, "2", time);
    EXPECT_TRUE(queue.push(std::make_pair(task1, 1)));
    EXPECT_TRUE(queue.push(std::make_pair(task2, 2)));
    queue.pop();
    auto frontTask = queue.front();
    EXPECT_EQ(frontTask.first, task2);
}

TEST(QueueTest, Empty) {
    TP::Queue queue(3);
    EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, NotEmpty) {
    TP::Queue queue(3);
    auto time = std::time_t{2};
    auto task1 = std::make_shared<TP::Task>(1, 2, "1", time);
    auto task2 = std::make_shared<TP::Task>(1, 2, "2", time);
    EXPECT_TRUE(queue.push(std::make_pair(task1, 1)));
    EXPECT_TRUE(queue.push(std::make_pair(task2, 2)));
    EXPECT_FALSE(queue.empty());
}

TEST(QueueTest, Duplication) {
    TP::Queue queue(3);
    auto time = std::time_t{2};
    auto task1 = std::make_shared<TP::Task>(1,2,"1", time);
    auto task2 = std::make_shared<TP::Task>(1,2,"1", time);


    EXPECT_TRUE(queue.push(std::make_pair(task1, 1)));
    EXPECT_TRUE(queue.push(std::make_pair(task2, 2)));
    auto fut = task1->promise_->get_future().get();
    EXPECT_EQ(fut.status, CallStatus::Duplication);
}