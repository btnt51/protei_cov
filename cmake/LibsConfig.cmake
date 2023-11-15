include(FetchContent)

function(DownloadBoostLightWeight GIT_TAG libraries)
    if(NOT libraries)
        message(FATAL "There is no provided libraries")
    endif()
    foreach(lib ${libraries})
        string(REPLACE "|" ";" lib "${lib}")
        list(GET lib 0 NAME)
        list(GET lib 1 URL)
        FetchContent_Declare(
            ${NAME}
            GIT_REPOSITORY ${URL}
            GIT_TAG ${GIT_TAG}
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(${NAME})
    endforeach()
    list(LENGTH libraries len)
    message("Amount of libraries: ${len}")
endfunction()


function(DownloadAndUseLibs LIGHTWEIGHTBABY)
    find_package(spdlog REQUIRED)
    if(NOT spdlog_FOUND)
        message("Downloading spdlog")
        FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG        v1.12.0
            GIT_SHALLOW TRUE
        )
        FetchContent_GetProperties(spdlog)
        if(NOT spdlog_POPULATED)
            FetchContent_Populate(spdlog)
            add_subdirectory(
                ${spdlog_SOURCE_DIR}
                ${spdlog_BINARY_DIR}
            )
        endif()
    else()
        message("Using system spdlog library")
    endif()

    find_package(Boost COMPONENTS system date_time REQUIRED)
    if(NOT Boost_FOUND)
        set(GIT_TAG boost-1.83.0)
        if(LIGHTWEIGHTBABY)
            message("Light Weight Baby")
            set(libraries
                "boost_mp11|https://github.com/boostorg/mp11"
                "boost_variant2|https://github.com/boostorg/variant2"
                "boost_predef|https://github.com/boostorg/predef"
                "boost_winapi|https://github.com/boostorg/winapi"
                "boost_throw_exception|https://github.com/boostorg/throw_exception"
                "boost_system|https://github.com/boostorg/system"
                "boost_config|https://github.com/boostorg/config"
                "boost_static_assert|https://github.com/boostorg/static_assert"
                "boost_assert|https://github.com/boostorg/assert"
                "boost_core|https://github.com/boostorg/core"
                "boost_align|https://github.com/boostorg/align"
                "boost_array|https://github.com/boostorg/array"
                "boost_bind|https://github.com/boostorg/bind"
                "boost_integer|https://github.com/boostorg/integer"
                "boost_move|https://github.com/boostorg/move"
                "boost_preprocessor|https://github.com/boostorg/preprocessor"
                "boost_type_traits|https://github.com/boostorg/type_traits"
                "boost_utility|https://github.com/boostorg/utility"
                "boost_io|https://github.com/boostorg/io"
                "boost_mpl|https://github.com/boostorg/mpl"
                "boost_rational|https://github.com/boostorg/rational"
                "boost_ratio|https://github.com/boostorg/ratio"
                "boost_typeof|https://github.com/boostorg/typeof"
                "boost_chrono|https://github.com/boostorg/chrono"
                "boost_pool|https://github.com/boostorg/pool"
                "boost_smart_ptr|https://github.com/boostorg/smart_ptr"
                "boost_context|https://github.com/boostorg/context"
                "boost_tuple|https://github.com/boostorg/tuple"
                "boost_exception|https://github.com/boostorg/exception"
                "boost_coroutine|https://github.com/boostorg/coroutine"
                "boost_concept_check|https://github.com/boostorg/concept_check"
                "boost_detail|https://github.com/boostorg/detail"
                "boost_function_types|https://github.com/boostorg/function_types"
                "boost_functional|https://github.com/boostorg/functional"
                "boost_fusion|https://github.com/boostorg/fusion"
                "boost_iterator|https://github.com/boostorg/iterator"
                "boost_unordered|https://github.com/boostorg/unordered"
                "boost_algorithm|https://github.com/boostorg/algorithm"
                "boost_lexical_cast|https://github.com/boostorg/lexical_cast"
                "boost_tokenizer|https://github.com/boostorg/tokenizer"
                "boost_date_time|https://github.com/boostorg/date_time"
                "boost_function|https://github.com/boostorg/function"
                "boost_logic|https://github.com/boostorg/logic"
                "boost_regex|https://github.com/boostorg/regex"
                "boost_asio|https://github.com/boostorg/asio"
                "boost_intrusive|https://github.com/boostorg/intrusive"
                "boost_container|https://github.com/boostorg/container"
                "boost_endian|https://github.com/boostorg/endian"
                "boost_optional|https://github.com/boostorg/optional"
                "boost_static_string|https://github.com/boostorg/static_string"
                "boost_beast|https://github.com/boostorg/beast"
                "boost_property_tree|https://github.com/boostorg/property_tree"
                "boost_describe|https://github.com/boostorg/describe"
                "boost_container_hash|https://github.com/boostorg/container_hash"
                "boost_conversion|https://github.com/boostorg/conversion"
                "boost_numeric_conversion|https://github.com/boostorg/numeric_conversion"
                "boost_dynamic_bitset|https://github.com/boostorg/dynamic_bitset"
                "boost_range|https://github.com/boostorg/range"
                "boost_random|https://github.com/boostorg/random"
                "boost_tti|https://github.com/boostorg/tti"
                "boost_uuid|https://github.com/boostorg/uuid"
            )
            DownloadBoostLightWeight(${GIT_TAG} "${libraries}")
        else()
            message("Heavy build")
            set(BOOST_ENABLE_CMAKE ON)
            set(BOOST_INCLUDE_LIBRARIES system asio date_time beast property_tree uuid)
            FetchContent_Declare(
                Boost
                GIT_REPOSITORY https://github.com/boostorg/boost.git
                GIT_TAG ${GIT_TAG}
                GIT_SHALLOW TRUE
            )
            FetchContent_MakeAvailable(Boost)
        endif()
        set(LinkLibraries spdlog::spdlog Boost::asio Boost::beast Boost::property_tree Boost::uuid PARENT_SCOPE)
    else()
        message("Using system Boost libraries.")
        set(LinkLibraries spdlog::spdlog PARENT_SCOPE)
    endif()

    set(LinkInclude spdlog::spdlog Boost::asio Boost::beast Boost::property_tree Boost::uuid PARENT_SCOPE)
endfunction()
