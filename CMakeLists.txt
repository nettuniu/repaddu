cmake_minimum_required(VERSION 3.20)

project(repaddu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(repaddu_core
    src/core_types.cpp
)

target_include_directories(repaddu_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(repaddu_core PUBLIC cxx_std_20)

add_library(repaddu_io
    src/io_traversal.cpp
    src/io_binary.cpp
)

target_include_directories(repaddu_io
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_io
    PUBLIC
        repaddu_core
)

target_compile_features(repaddu_io PUBLIC cxx_std_20)

add_library(repaddu_grouping
    src/grouping_strategies.cpp
    src/grouping_component_map.cpp
)

target_include_directories(repaddu_grouping
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_grouping
    PUBLIC
        repaddu_core
)

target_compile_features(repaddu_grouping PUBLIC cxx_std_20)

add_library(repaddu_format
    src/format_writer.cpp
    src/format_tree.cpp
)

target_include_directories(repaddu_format
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_format
    PUBLIC
        repaddu_core
)

target_compile_features(repaddu_format PUBLIC cxx_std_20)

add_library(repaddu_cli
    src/cli_parse.cpp
    src/cli_run.cpp
)

target_include_directories(repaddu_cli
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_cli
    PUBLIC
        repaddu_core
        repaddu_io
        repaddu_grouping
        repaddu_format
)

target_compile_features(repaddu_cli PUBLIC cxx_std_20)

add_executable(repaddu
    src/main.cpp
)

target_link_libraries(repaddu
    PRIVATE
        repaddu_cli
)

enable_testing()

add_executable(repaddu_test_filtering
    tests/test_filtering.cpp
)

target_include_directories(repaddu_test_filtering
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_filtering
    PRIVATE
        repaddu_core
        repaddu_io
        repaddu_grouping
        repaddu_format
)

target_compile_features(repaddu_test_filtering PRIVATE cxx_std_20)
target_compile_definitions(repaddu_test_filtering PRIVATE REPADDU_TEST_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\")

add_executable(repaddu_test_grouping
    tests/test_grouping.cpp
)

target_include_directories(repaddu_test_grouping
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_grouping
    PRIVATE
        repaddu_core
        repaddu_io
        repaddu_grouping
        repaddu_format
)

target_compile_features(repaddu_test_grouping PRIVATE cxx_std_20)
target_compile_definitions(repaddu_test_grouping PRIVATE REPADDU_TEST_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\")

add_executable(repaddu_test_cmake
    tests/test_cmake_aggregation.cpp
)

target_include_directories(repaddu_test_cmake
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_cmake
    PRIVATE
        repaddu_core
        repaddu_io
        repaddu_grouping
        repaddu_format
)

target_compile_features(repaddu_test_cmake PRIVATE cxx_std_20)
target_compile_definitions(repaddu_test_cmake PRIVATE REPADDU_TEST_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\")

add_test(NAME repaddu_test_filtering COMMAND repaddu_test_filtering)
add_test(NAME repaddu_test_grouping COMMAND repaddu_test_grouping)
add_test(NAME repaddu_test_cmake COMMAND repaddu_test_cmake)
