cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

set(PROJECT_NAME protei_cov)

project(${PROJECT_NAME} VERSION ${PROJECT_VESRION})

set(SOURCES src/jsonParser.cpp src/config.cpp src/threadpool.cpp src/manager.cpp)
set(HEADERS include/config.hpp include/commonStructures.hpp
        include/jsonParser.hpp include/recorder.hpp include/threadpool.hpp
        include/manager.hpp include/interfaces.hpp)
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
)

##download libraries and include them
##
include(cmake/LibsConfig.cmake)
DownloadAndUseLibs(LIGHTWEIGHTBABY)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LinkLibraries})
target_include_directories(${PROJECT_NAME} PRIVATE ${LinkInclude})
##
##end of downloading libraries and including them
##fetching tests
##
add_definitions(-DWITH_TESTS)
if(WITH_TESTS)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.13.0
    )
    FetchContent_MakeAvailable(googletest)
    add_library(GTest::GTest INTERFACE IMPORTED)
    target_link_libraries(GTest::GTest INTERFACE gtest_main gmock_main)
    add_executable(test tests/managerTests.cpp ${SOURCES})
    target_compile_features(test PUBLIC cxx_std_20)
    target_link_libraries(test GTest::GTest ${LinkLibraries})
    target_include_directories(test PRIVATE include)
    add_custom_command(
            TARGET ${PROJECT_NAME}  POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/base.json
            ${CMAKE_CURRENT_BINARY_DIR}/)

    include(GoogleTest)
    gtest_discover_tests(test)
endif()
##
##end fetching tests


if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
    )
    if(BUILDING_TESTS)
        target_compile_options(${PROJECT_NAME}_test PRIVATE
            /W4
        )
    endif()
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    if(BUILDING_TESTS)
        target_compile_options(${PROJECT_NAME}_test PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
    endif()
endif()
