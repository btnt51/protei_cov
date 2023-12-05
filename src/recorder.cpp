#include "recorder.hpp"

FileRecorder::FileRecorder(const std::filesystem::path& path) {
    file = std::ofstream (path, std::ios::out|std::ios::app);
    std::string newL(136, '#');
    file << newL << std::endl;
    file << "#DT of the incoming call;Incoming Call ID;Caller number;DT of call termination;Call status;";
    file << "DT operator answer;Operator ID;Call duration#" << std::endl;
    file << newL << std::endl;
}

void FileRecorder::makeRecord(const CDR& cdr) {
    std::lock_guard<std::mutex> lockGuard(writeMutex_);
    file << cdr << std::endl;
}

void FileRecorder::setLogger(std::shared_ptr<spdlog::logger> logger) {
    logger_ = logger;
}
