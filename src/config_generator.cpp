#include "repaddu/config_generator.h"
#include <fstream>

namespace repaddu::config
    {
    core::RunResult generateDefaultConfig(const std::filesystem::path& path)
        {
        if (std::filesystem::exists(path))
            {
            return { core::ExitCode::io_failure, "Config file already exists: " + path.string() };
            }

        std::ofstream ofs(path);
        if (!ofs)
            {
            return { core::ExitCode::io_failure, "Failed to create config file: " + path.string() };
            }

        // We manually format the JSON to keep it readable and ordered without a library
        ofs << "{\n";
        ofs << "    \"input\": \".\",\n";
        ofs << "    \"output\": \"repaddu_out\",\n";
        ofs << "    \"max_files\": 0,\n";
        ofs << "    \"max_bytes\": 0,\n";
        ofs << "    \"number_width\": 3,\n";
        ofs << "    \"include_headers\": false,\n";
        ofs << "    \"include_sources\": true,\n";
        ofs << "    \"include_hidden\": false,\n";
        ofs << "    \"include_binaries\": false,\n";
        ofs << "    \"follow_symlinks\": false,\n";
        ofs << "    \"headers_first\": false,\n";
        ofs << "    \"emit_tree\": true,\n";
        ofs << "    \"emit_cmake\": true,\n";
        ofs << "    \"emit_build_files\": false,\n";
        ofs << "    \"frontmatter\": false,\n";
        ofs << "    \"max_file_size\": 1048576,\n";
        ofs << "    \"force_large\": false,\n";
        ofs << "    \"redact_pii\": false,\n";
        ofs << "    \"analyze_only\": false,\n";
        ofs << "    \"analysis_enabled\": false,\n";
        ofs << "    \"analysis_views\": [],\n";
        ofs << "    \"analysis_deep\": false,\n";
        ofs << "    \"analysis_collapse\": \"none\",\n";
        ofs << "    \"extract_tags\": false,\n";
        ofs << "    \"tag_patterns\": \"\",\n";
        ofs << "    \"isolate_docs\": false,\n";
        ofs << "    \"dry_run\": false,\n";
        ofs << "    \"parallel_traversal\": true,\n";
        ofs << "    \"format\": \"markdown\",\n";
        ofs << "    \"group_by\": \"directory\",\n";
        ofs << "    \"markers\": \"fenced\",\n";
        ofs << "    \"extensions\": [],\n";
        ofs << "    \"exclude_extensions\": []\n";
        ofs << "}\n";

        return { core::ExitCode::success, "Generated default config: " + path.string() };
        }
    }
