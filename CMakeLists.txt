cmake_minimum_required(VERSION 3.24)
set(PATCH_VERSION "1" CACHE STRING "Patch version")
project(protei_cov VERSION 0.0.${PATCH_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
option(WITH_TESTS "Build unit and HTTP smoke tests" ON)
option(WARNINGS_AS_ERRORS "Treat project warnings as errors" ON)

function(target_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

include(cmake/LibsConfig.cmake)
add_subdirectory(protei_cov_core)
add_executable(protei_cov src/main.cpp src/httpServer.cpp)
target_include_directories(protei_cov PRIVATE include)
target_link_libraries(protei_cov PRIVATE protei_cov_core)
target_warnings(protei_cov)
configure_file(tests/base.json base.json COPYONLY)

if(WITH_TESTS)
    enable_testing()
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
    add_executable(protei_cov_tests
        tests/managerTests.cpp tests/configTests.cpp tests/queueTests.cpp
        tests/builderTests.cpp tests/jsonParserTests.cpp tests/commonStructuresTests.cpp
    )
    target_link_libraries(protei_cov_tests PRIVATE
        protei_cov_core GTest::gtest_main GTest::gmock)
    target_warnings(protei_cov_tests)
    foreach(fixture base lowerBorder upperBorder broken)
        configure_file(tests/${fixture}.json ${fixture}.json COPYONLY)
    endforeach()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME HttpSmoke COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/http_smoke.py" "$<TARGET_FILE:protei_cov>")
    set_tests_properties(HttpSmoke PROPERTIES TIMEOUT 20 RUN_SERIAL TRUE)
    include(GoogleTest)
    # Tests write shared log/output files; serialize them even with ctest -j.
    gtest_discover_tests(protei_cov_tests
        PROPERTIES ENVIRONMENT "TZ=UTC0" RUN_SERIAL TRUE TIMEOUT 15)
endif()
