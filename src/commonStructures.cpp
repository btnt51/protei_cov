#include "commonStructures.hpp"

/** @file commonStructures.cpp
 *  @brief Содержит определение функций необходимых для работы
 *  с CallStatus и CDR.
 */

namespace utility {
using std::string_literals::operator""s;
std::string to_string(const CallStatus& status) {
    switch (status) {
    case CallStatus::Awaiting: return "Awaiting"s; break;
    case CallStatus::Completed: return "Completed"s; break;
    case CallStatus::Duplication: return "Duplication"s; break;
    case CallStatus::Overloaded: return "Overloaded"s; break;
    case CallStatus::Rejected: return "Rejected"s; break;
    case CallStatus::Timeout: return "Timeout"s; break;
    }
    return "Null"s;
}

using system_clock = std::chrono::system_clock;
using time_point = system_clock::time_point;
using seconds = std::chrono::seconds;
using milliseconds = std::chrono::milliseconds;


std::string prepareTime(const time_point& timeP) {
    if (timeP == time_point()) {
        return ""s;
    } else {
        auto timeSinceEpoch = timeP.time_since_epoch();
        auto seconds_ = std::chrono::duration_cast<seconds>(timeSinceEpoch);
        auto milliseconds_ = std::chrono::duration_cast<milliseconds>(timeSinceEpoch - seconds_);

        std::time_t time = system_clock::to_time_t(timeP);
        std::tm timeInfo = *std::localtime(&time);

        std::ostringstream oss;
        oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0')
            << milliseconds_.count();

        return oss.str();
    }
}
}

std::ofstream& operator<<(std::ofstream& out, const CDR& cdr) {
    out << utility::prepareTime(cdr.startTime) << ";" << cdr.callID << ";" << std::string{cdr.number} << ";" << utility::prepareTime(cdr.endTime) << ";";
    out << utility::to_string(cdr.status) << ";" << utility::prepareTime(cdr.operatorCallTime) << ";" << cdr.operatorID << ";";
    out << cdr.callDuration.count() << "s";
    return out;
}