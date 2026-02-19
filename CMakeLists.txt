cmake_minimum_required(VERSION 3.20)

project(repaddu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(REPADDU_ENABLE_CLANG "Enable Clang-based C++ analysis" OFF)

add_library(repaddu_base
    src/core_types.cpp
    src/language_profiles.cpp
    src/logger.cpp
    src/pii_redactor.cpp
    src/analysis_tokens.cpp
    src/analysis_tags.cpp
    src/json_lite.cpp
)

target_include_directories(repaddu_base
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(repaddu_base PUBLIC cxx_std_20)

add_library(repaddu_analysis
    src/analysis_graph.cpp
    src/analysis_view.cpp
    src/analysis_lsp.cpp
)

target_include_directories(repaddu_analysis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_analysis
    PUBLIC
        repaddu_base
)

target_compile_features(repaddu_analysis PUBLIC cxx_std_20)

# Compatibility aggregate target retained for existing links/tests.
add_library(repaddu_core INTERFACE)

target_link_libraries(repaddu_core
    INTERFACE
        repaddu_base
        repaddu_analysis
)

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
        repaddu_base
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
        repaddu_base
)

target_compile_features(repaddu_grouping PUBLIC cxx_std_20)

add_library(repaddu_format
    src/format_writer.cpp
    src/format_writer_read.cpp
    src/format_writer_alt_formats.cpp
    src/format_tree.cpp
    src/format_language_report.cpp
    src/format_analysis_report.cpp
    src/format/analysis_tags_report.cpp
    src/format_analysis_json.cpp
)

target_include_directories(repaddu_format
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_format
    PUBLIC
        repaddu_base
        repaddu_analysis
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
    src/app/analysis_backend_default.cpp
    src/app/app_analyze.cpp
    src/app/effective_options.cpp
    src/app/fs_services.cpp
    src/cli_bootstrap.cpp
    src/app_run.cpp
    src/cli_parse.cpp
    src/cli_parse_values.cpp
    src/cli_parse_config.cpp
    src/cli_parse_help.cpp
    src/cli_run.cpp
    src/config_generator.cpp
)

target_include_directories(repaddu_cli
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(repaddu_cli
    PUBLIC
        repaddu_base
        repaddu_analysis
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
                repaddu_analysis
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

function(repaddu_add_test target source)
    set(options WITH_TEST_ROOT)
    set(oneValueArgs)
    set(multiValueArgs LIBS)
    cmake_parse_arguments(REPADDU_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${target}
        ${source}
    )

    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    if (REPADDU_TEST_LIBS)
        target_link_libraries(${target}
            PRIVATE
                ${REPADDU_TEST_LIBS}
        )
    endif()

    target_compile_features(${target} PRIVATE cxx_std_20)

    if (REPADDU_TEST_WITH_TEST_ROOT)
        target_compile_definitions(${target} PRIVATE REPADDU_TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

add_test(NAME ${target} COMMAND ${target})
endfunction()

if (UNIX)
    add_test(NAME repaddu_test_dependency_policy
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_component_deps.sh)
endif()

repaddu_add_test(repaddu_test_filtering tests/test_filtering.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io repaddu_grouping repaddu_format
)

repaddu_add_test(repaddu_test_grouping tests/test_grouping.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io repaddu_grouping repaddu_format
)

repaddu_add_test(repaddu_test_isolate_docs tests/test_isolate_docs.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io repaddu_grouping
)

repaddu_add_test(repaddu_test_cmake tests/test_cmake_aggregation.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io repaddu_grouping repaddu_format
)

repaddu_add_test(repaddu_test_logging tests/test_logging.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_binary tests/test_binary_detection.cpp
    LIBS repaddu_io
)

repaddu_add_test(repaddu_test_large_file tests/test_large_file.cpp
    LIBS repaddu_grouping
)

repaddu_add_test(repaddu_test_pii tests/test_pii.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_tokens tests/test_tokens.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_tags tests/test_tags.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_ui tests/test_ui.cpp
    LIBS repaddu_ui
)

repaddu_add_test(repaddu_test_analysis_graph tests/test_analysis_graph.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_analysis_views tests/test_analysis_views.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_cli_parse_analysis tests/test_cli_parse_analysis.cpp
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_config_analysis tests/test_config_analysis.cpp
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_cli_config_path tests/test_cli_config_path.cpp
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_dry_run tests/test_dry_run.cpp
    WITH_TEST_ROOT
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_app_run tests/test_app_run.cpp
    WITH_TEST_ROOT
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_analysis_json tests/test_analysis_json.cpp
    LIBS repaddu_format
)

repaddu_add_test(repaddu_test_analysis tests/test_analysis.cpp
    WITH_TEST_ROOT
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_frontmatter_output tests/test_frontmatter_output.cpp
    WITH_TEST_ROOT
    LIBS repaddu_format
)

repaddu_add_test(repaddu_test_analysis_tags_report tests/test_analysis_tags_report.cpp
    LIBS repaddu_cli
)

repaddu_add_test(repaddu_test_fixture_picobench tests/test_fixture_picobench.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io repaddu_format
)

repaddu_add_test(repaddu_test_language_profiles tests/test_language_profiles_detection.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_multilanguage_fixture tests/test_multilanguage_fixture.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io repaddu_grouping
)

repaddu_add_test(repaddu_test_concurrency tests/test_concurrency.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core repaddu_io
)

repaddu_add_test(repaddu_test_lsp_client tests/test_lsp_client.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_lsp_symbols tests/test_lsp_symbols.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_lsp_relationships tests/test_lsp_relationships.cpp
    LIBS repaddu_core
)

repaddu_add_test(repaddu_test_analysis_lsp tests/test_analysis_lsp.cpp
    WITH_TEST_ROOT
    LIBS repaddu_core
)
