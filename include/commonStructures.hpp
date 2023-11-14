#pragma once
#include <chrono>
#include <thread>

namespace TP {
    using CallID = long long;
}

enum class CallStatus {
    awaiting,
    completed,
    rejected
};


struct CDR {
    std::time_t startTime;
    std::time_t endTime;
    TP::CallID callID;
    CallStatus status;
    std::thread::id operatorID;
    std::chrono::duration<int> callDuration;
    long long number;
};