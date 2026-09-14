add_library(protei_cov_core STATIC
        src/manager.cpp
        src/threadpool.cpp
        src/queue.cpp
        src/task.cpp
        src/config.cpp
        src/jsonParser.cpp
        src/recorder.cpp
        src/commonStructures.cpp
        src/builder.cpp
)

target_include_directories(protei_cov_core
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(protei_cov_core
        PUBLIC spdlog::spdlog_header_only Boost::headers Threads::Threads
)
target_compile_features(protei_cov_core PUBLIC cxx_std_20)
target_warnings(protei_cov_core)
