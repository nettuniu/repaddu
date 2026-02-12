cmake_minimum_required(VERSION 3.20)

project(repaddu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(REPADDU_ENABLE_CLANG "Enable Clang-based C++ analysis" OFF)

add_library(repaddu_core
    src/core_types.cpp
    src/language_profiles.cpp
    src/logger.cpp
    src/pii_redactor.cpp
    src/analysis_tokens.cpp
    src/analysis_tags.cpp
    src/analysis_graph.cpp
    src/analysis_view.cpp
    src/analysis_lsp.cpp
    src/json_lite.cpp
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
    src/format_language_report.cpp
    src/format_analysis_report.cpp
    src/format_analysis_json.cpp
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

add_library(repaddu_ui
    src/ui_console.cpp
)

target_include_directories(repaddu_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(repaddu_ui PUBLIC cxx_std_20)

add_library(repaddu_cli
    src/cli_parse.cpp
    src/cli_run.cpp
    src/config_generator.cpp
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
        repaddu_ui
)

target_compile_features(repaddu_cli PUBLIC cxx_std_20)

if (REPADDU_ENABLE_CLANG)
    set(REPADDU_ENABLE_CLANG_TARGETS ON)

    find_program(REPADDU_LLVM_CONFIG_EXECUTABLE NAMES llvm-config llvm-config-18)
    if (REPADDU_LLVM_CONFIG_EXECUTABLE)
        execute_process(
            COMMAND ${REPADDU_LLVM_CONFIG_EXECUTABLE} --libdir
            OUTPUT_VARIABLE REPADDU_LLVM_LIBDIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if (REPADDU_LLVM_LIBDIR AND NOT EXISTS "${REPADDU_LLVM_LIBDIR}/libclangBasic.a")
            message(WARNING
                "REPADDU_ENABLE_CLANG=ON requested, but libclangBasic.a was not found in ${REPADDU_LLVM_LIBDIR}. "
                "Skipping Clang analyzer targets.")
            set(REPADDU_ENABLE_CLANG_TARGETS OFF)
        endif()
    endif()

    if (REPADDU_ENABLE_CLANG_TARGETS)
        find_package(LLVM REQUIRED CONFIG)
        find_package(Clang REQUIRED CONFIG)

        add_library(repaddu_cpp_analyzer
            src/analysis_cpp.cpp
        )

        target_include_directories(repaddu_cpp_analyzer
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${LLVM_INCLUDE_DIRS}
        )

        target_link_libraries(repaddu_cpp_analyzer
            PUBLIC
                repaddu_core
                clangTooling
                clangAST
                clangASTMatchers
                clangFrontend
                clangBasic
        )

        target_compile_features(repaddu_cpp_analyzer PUBLIC cxx_std_20)
        target_compile_definitions(repaddu_cpp_analyzer PUBLIC REPADDU_ENABLE_CLANG=1)

        add_executable(repaddu_test_analysis_cpp
            tests/test_analysis_cpp.cpp
        )

        target_include_directories(repaddu_test_analysis_cpp
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

        target_link_libraries(repaddu_test_analysis_cpp
            PRIVATE
                repaddu_cpp_analyzer
        )

        target_compile_features(repaddu_test_analysis_cpp PRIVATE cxx_std_20)
        target_compile_definitions(repaddu_test_analysis_cpp PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

        add_test(NAME repaddu_test_analysis_cpp COMMAND repaddu_test_analysis_cpp)
    endif()
endif()

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
target_compile_definitions(repaddu_test_filtering PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

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
target_compile_definitions(repaddu_test_grouping PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

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
target_compile_definitions(repaddu_test_cmake PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(repaddu_test_logging
    tests/test_logging.cpp
)

target_include_directories(repaddu_test_logging
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_logging
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_logging PRIVATE cxx_std_20)

add_executable(repaddu_test_binary
    tests/test_binary_detection.cpp
)

target_include_directories(repaddu_test_binary
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_binary
    PRIVATE
        repaddu_io
)

target_compile_features(repaddu_test_binary PRIVATE cxx_std_20)

add_executable(repaddu_test_large_file
    tests/test_large_file.cpp
)

target_include_directories(repaddu_test_large_file
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_large_file
    PRIVATE
        repaddu_grouping
)

target_compile_features(repaddu_test_large_file PRIVATE cxx_std_20)

add_executable(repaddu_test_pii
    tests/test_pii.cpp
)

target_include_directories(repaddu_test_pii
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_pii
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_pii PRIVATE cxx_std_20)

add_executable(repaddu_test_tokens
    tests/test_tokens.cpp
)

target_include_directories(repaddu_test_tokens
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_tokens
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_tokens PRIVATE cxx_std_20)

add_executable(repaddu_test_tags
    tests/test_tags.cpp
)

target_include_directories(repaddu_test_tags
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_tags
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_tags PRIVATE cxx_std_20)

add_executable(repaddu_test_ui
    tests/test_ui.cpp
)

target_include_directories(repaddu_test_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_ui
    PRIVATE
        repaddu_ui
)

target_compile_features(repaddu_test_ui PRIVATE cxx_std_20)

add_executable(repaddu_test_analysis_graph
    tests/test_analysis_graph.cpp
)

target_include_directories(repaddu_test_analysis_graph
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_analysis_graph
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_analysis_graph PRIVATE cxx_std_20)

add_executable(repaddu_test_analysis_views
    tests/test_analysis_views.cpp
)

target_include_directories(repaddu_test_analysis_views
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_analysis_views
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_analysis_views PRIVATE cxx_std_20)

add_executable(repaddu_test_cli_parse_analysis
    tests/test_cli_parse_analysis.cpp
)

target_include_directories(repaddu_test_cli_parse_analysis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_cli_parse_analysis
    PRIVATE
        repaddu_cli
)

target_compile_features(repaddu_test_cli_parse_analysis PRIVATE cxx_std_20)

add_executable(repaddu_test_config_analysis
    tests/test_config_analysis.cpp
)

target_include_directories(repaddu_test_config_analysis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_config_analysis
    PRIVATE
        repaddu_cli
)

target_compile_features(repaddu_test_config_analysis PRIVATE cxx_std_20)

add_executable(repaddu_test_analysis_json
    tests/test_analysis_json.cpp
)

target_include_directories(repaddu_test_analysis_json
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_analysis_json
    PRIVATE
        repaddu_format
)

target_compile_features(repaddu_test_analysis_json PRIVATE cxx_std_20)

add_executable(repaddu_test_fixture_picobench
    tests/test_fixture_picobench.cpp
)

target_include_directories(repaddu_test_fixture_picobench
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_fixture_picobench
    PRIVATE
        repaddu_core
        repaddu_io
        repaddu_format
)

target_compile_features(repaddu_test_fixture_picobench PRIVATE cxx_std_20)
target_compile_definitions(repaddu_test_fixture_picobench PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(repaddu_test_language_profiles
    tests/test_language_profiles_detection.cpp
)

target_include_directories(repaddu_test_language_profiles
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_language_profiles
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_language_profiles PRIVATE cxx_std_20)

add_executable(repaddu_test_multilanguage_fixture
    tests/test_multilanguage_fixture.cpp
)

target_include_directories(repaddu_test_multilanguage_fixture
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_multilanguage_fixture
    PRIVATE
        repaddu_core
        repaddu_io
        repaddu_grouping
)

target_compile_features(repaddu_test_multilanguage_fixture PRIVATE cxx_std_20)
target_compile_definitions(repaddu_test_multilanguage_fixture PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(repaddu_test_lsp_client
    tests/test_lsp_client.cpp
)

target_include_directories(repaddu_test_lsp_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_lsp_client
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_lsp_client PRIVATE cxx_std_20)

add_executable(repaddu_test_lsp_symbols
    tests/test_lsp_symbols.cpp
)

target_include_directories(repaddu_test_lsp_symbols
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_lsp_symbols
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_lsp_symbols PRIVATE cxx_std_20)

add_executable(repaddu_test_lsp_relationships
    tests/test_lsp_relationships.cpp
)

target_include_directories(repaddu_test_lsp_relationships
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_lsp_relationships
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_lsp_relationships PRIVATE cxx_std_20)

add_executable(repaddu_test_analysis_lsp
    tests/test_analysis_lsp.cpp
)

target_include_directories(repaddu_test_analysis_lsp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_test_analysis_lsp
    PRIVATE
        repaddu_core
)

target_compile_features(repaddu_test_analysis_lsp PRIVATE cxx_std_20)
target_compile_definitions(repaddu_test_analysis_lsp PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME repaddu_test_filtering COMMAND repaddu_test_filtering)
add_test(NAME repaddu_test_grouping COMMAND repaddu_test_grouping)
add_test(NAME repaddu_test_cmake COMMAND repaddu_test_cmake)
add_test(NAME repaddu_test_logging COMMAND repaddu_test_logging)
add_test(NAME repaddu_test_binary COMMAND repaddu_test_binary)
add_test(NAME repaddu_test_large_file COMMAND repaddu_test_large_file)
add_test(NAME repaddu_test_pii COMMAND repaddu_test_pii)
add_test(NAME repaddu_test_tokens COMMAND repaddu_test_tokens)
add_test(NAME repaddu_test_tags COMMAND repaddu_test_tags)
add_test(NAME repaddu_test_ui COMMAND repaddu_test_ui)
add_test(NAME repaddu_test_analysis_graph COMMAND repaddu_test_analysis_graph)
add_test(NAME repaddu_test_analysis_views COMMAND repaddu_test_analysis_views)
add_test(NAME repaddu_test_cli_parse_analysis COMMAND repaddu_test_cli_parse_analysis)
add_test(NAME repaddu_test_config_analysis COMMAND repaddu_test_config_analysis)
add_test(NAME repaddu_test_analysis_json COMMAND repaddu_test_analysis_json)
add_test(NAME repaddu_test_fixture_picobench COMMAND repaddu_test_fixture_picobench)
add_test(NAME repaddu_test_language_profiles COMMAND repaddu_test_language_profiles)
add_test(NAME repaddu_test_multilanguage_fixture COMMAND repaddu_test_multilanguage_fixture)
add_test(NAME repaddu_test_lsp_client COMMAND repaddu_test_lsp_client)
add_test(NAME repaddu_test_lsp_symbols COMMAND repaddu_test_lsp_symbols)
add_test(NAME repaddu_test_lsp_relationships COMMAND repaddu_test_lsp_relationships)
add_test(NAME repaddu_test_analysis_lsp COMMAND repaddu_test_analysis_lsp)
