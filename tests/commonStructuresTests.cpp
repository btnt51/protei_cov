#include "commonStructures.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

TEST(CommonStructures, ReturnsCorrectStdStringForCallStatus) {
    EXPECT_EQ(utility::to_string(CallStatus::Awaiting), std::string{"Awaiting"});
    EXPECT_EQ(utility::to_string(CallStatus::Completed), std::string{"Completed"});
    EXPECT_EQ(utility::to_string(CallStatus::Duplication), std::string{"Duplication"});
    EXPECT_EQ(utility::to_string(CallStatus::Overloaded), std::string{"Overloaded"});
    EXPECT_EQ(utility::to_string(CallStatus::Rejected), std::string{"Rejected"});
    EXPECT_EQ(utility::to_string(CallStatus::Timeout), std::string{"Timeout"});
}

TEST(CommonStructures, ReturnsCorrectFormattedTimeString) {
    std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now();
    std::string formattedTime = utility::prepareTime(timePoint);

    EXPECT_TRUE(std::regex_match(formattedTime, std::regex("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}")));
}

TEST(CommonStructures, CorrectlyWritesCDRToOfstream) {
    CDR cdr;
    cdr.startTime = std::chrono::system_clock::from_time_t(std::time_t{1701775845});
    cdr.operatorCallTime = std::chrono::system_clock::from_time_t(std::time_t{1701775846});
    cdr.endTime = std::chrono::system_clock::from_time_t(std::time_t{1701775856});
    cdr.number = "2134";
    cdr.callDuration = std::chrono::seconds(10);
    cdr.status = CallStatus::Completed;
    cdr.callID = 123456789;
    cdr.operatorID = 12;

    std::ofstream outFile("test_output.txt");
    outFile << cdr;
    outFile.close();


    std::ifstream inFile("test_output.txt");
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    std::string expectedContent;
    if(std::chrono::current_zone()->name() == "Europe/Moscow")
        expectedContent = "2023-12-05 14:30:45.000;123456789;2134;2023-12-05 14:30:56.000;Completed;2023-12-05 14:30:46.000;12;10s";
    else
        expectedContent = "2023-12-05 11:30:45.000;123456789;2134;2023-12-05 11:30:56.000;Completed;2023-12-05 11:30:46.000;12;10s";
    EXPECT_EQ(content, expectedContent);


    std::remove("test_output.txt");
}

TEST(CommonStructures, CorrectlyWritesCDRToOfstreamWhenTimeout) {
    CDR cdr;
    cdr.startTime = std::chrono::system_clock::from_time_t(std::time_t{1701775845});
    cdr.operatorCallTime = std::chrono::system_clock::from_time_t(std::time_t{1701775846});
    cdr.endTime;
    cdr.number = "2134";
    cdr.callDuration = std::chrono::seconds(0);
    cdr.status = CallStatus::Timeout;
    cdr.callID = 123456789;
    cdr.operatorID = 12;

    std::ofstream outFile("test_output.txt");
    outFile << cdr;
    outFile.close();

    std::ifstream inFile("test_output.txt");
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    std::string expectedContent;
    if(std::chrono::current_zone()->name() == "Europe/Moscow")
        expectedContent = "2023-12-05 14:30:45.000;123456789;2134;;Timeout;2023-12-05 14:30:46.000;12;0s";
    else
        expectedContent = "2023-12-05 11:30:45.000;123456789;2134;;Timeout;2023-12-05 11:30:46.000;12;0s";


    EXPECT_EQ(content, expectedContent);

    std::remove("test_output.txt");
}