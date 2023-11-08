cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

set(PROJECT_NAME protei_cov)

project(${PROJECT_NAME} VERSION ${PROJECT_VESRION})

add_executable(${PROJECT_NAME} src/main.cpp src/jsonParser.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
)
##fetching libraries
##
message("Downloading spdlog")
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.12.0
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(
        ${spdlog_SOURCE_DIR}
        ${spdlog_BINARY_DIR}
    )
endif()

message("Downloading boost")
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES filesystem system asio date_time thread beast property_tree)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.83.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Boost)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog Boost::asio Boost::beast Boost::filesystem Boost::property_tree)
target_include_directories(${PROJECT_NAME} PRIVATE spdlog::spdlog Boost::asio Boost::beast Boost::filesystem Boost::property_tree)
##
##end fecthing libraries

##fetching tests
##
if(WITH_TESTS)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.14.0
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(test tests/integer.cpp tests/containers.cpp tests/strings.cpp)
    target_compile_features(test PUBLIC cxx_std_17)
    target_link_libraries(test GTest::gtest_main)
    target_include_directories(test PRIVATE include)

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


if (APPLE)
    message("Packing for MacOS")
    set(CPACK_GENERATOR DragNDrop)
elseif (UNIX)
    message("Packing for Debian distros")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "async")
    set(CPACK_PACKAGE_CONTACT btnt51@yande.ru)
endif ()