#include "repaddu/cli_parse.h"

#include "repaddu/core_types.h"
#include "repaddu/json_lite.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>

namespace repaddu::cli
    {
    std::filesystem::path resolveConfigPath(const std::vector<std::string>& args)
        {
        for (std::size_t i = 1; i + 1 < args.size(); ++i)
            {
            if (args[i] == "--config")
                {
                return args[i + 1];
                }
            }

        if (std::filesystem::exists(".repaddu.json"))
            {
            return ".repaddu.json";
            }
        if (std::filesystem::exists(".repaddu.yaml"))
            {
            return ".repaddu.yaml";
            }
        if (std::filesystem::exists(".repaddu.yml"))
            {
            return ".repaddu.yml";
            }
        return ".repaddu.json";
        }

    core::RunResult loadConfigFile(const std::filesystem::path& path, core::CliOptions& opt)
        {
        std::ifstream ifs(path);
        if (!ifs)
            {
            return { core::ExitCode::io_failure, "Failed to open config file: " + path.string() };
            }

        std::ostringstream buffer;
        buffer << ifs.rdbuf();

        auto trim = [](const std::string& value)
            {
            std::size_t start = 0;
            while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0)
                {
                ++start;
                }
            std::size_t end = value.size();
            while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0)
                {
                --end;
                }
            return value.substr(start, end - start);
            };

        auto unquote = [&](const std::string& value)
            {
            if (value.size() >= 2
                && ((value.front() == '"' && value.back() == '"')
                    || (value.front() == '\'' && value.back() == '\'')))
                {
                return value.substr(1, value.size() - 2);
                }
            return value;
            };

        auto parseConfigCommon = [&](auto&& getBool, auto&& getString, auto&& getPath,
                                     auto&& getInt, auto&& getUInt64, auto&& getStringArray)
            {
            getPath("input", opt.inputPath);
            getPath("output", opt.outputPath);
            getInt("max_files", opt.maxFiles);
            getUInt64("max_bytes", opt.maxBytes);
            getInt("number_width", opt.numberWidth);
            getBool("include_headers", opt.includeHeaders);
            getBool("include_sources", opt.includeSources);
            getBool("include_hidden", opt.includeHidden);
            getBool("include_binaries", opt.includeBinaries);
            getBool("follow_symlinks", opt.followSymlinks);
            getBool("headers_first", opt.headersFirst);
            getBool("emit_tree", opt.emitTree);
            getBool("emit_cmake", opt.emitCMake);
            getBool("emit_build_files", opt.emitBuildFiles);
            getBool("emit_links", opt.emitLinks);
            getBool("frontmatter", opt.emitFrontmatter);
            getUInt64("max_file_size", opt.maxFileSize);
            getBool("force_large", opt.forceLargeFiles);
            getBool("redact_pii", opt.redactPii);
            getBool("analyze_only", opt.analyzeOnly);
            getBool("analysis_enabled", opt.analysisEnabled);
            getString("analysis_collapse", opt.analysisCollapse);
            getBool("analysis_deep", opt.analysisDeep);
            getStringArray("analysis_views", opt.analysisViews);
            getBool("extract_tags", opt.extractTags);
            getPath("tag_patterns", opt.tagPatternsPath);
            getBool("isolate_docs", opt.isolateDocs);
            getBool("dry_run", opt.dryRun);
            getBool("parallel_traversal", opt.parallelTraversal);
            getStringArray("extensions", opt.extensions);
            getStringArray("exclude_extensions", opt.excludeExtensions);

            std::string value;
            getString("group_by", value);
            if (value == "directory") opt.groupBy = core::GroupingMode::directory;
            else if (value == "component") opt.groupBy = core::GroupingMode::component;
            else if (value == "type") opt.groupBy = core::GroupingMode::type;
            else if (value == "size") opt.groupBy = core::GroupingMode::size;

            value.clear();
            getString("format", value);
            if (value == "markdown") opt.format = core::OutputFormat::markdown;
            else if (value == "jsonl") opt.format = core::OutputFormat::jsonl;
            else if (value == "html") opt.format = core::OutputFormat::html;

            value.clear();
            getString("markers", value);
            if (value == "fenced") opt.markers = core::MarkerMode::fenced;
            else if (value == "sentinel") opt.markers = core::MarkerMode::sentinel;
            };

        const std::string extension = core::toLowerCopy(path.extension().string());
        if (extension == ".yaml" || extension == ".yml")
            {
            std::map<std::string, std::string> scalars;
            std::map<std::string, std::vector<std::string>> arrays;

            std::istringstream stream(buffer.str());
            std::string line;
            while (std::getline(stream, line))
                {
                std::string noInlineComment;
                noInlineComment.reserve(line.size());
                bool inSingleQuote = false;
                bool inDoubleQuote = false;
                bool escapedInDoubleQuote = false;
                for (char ch : line)
                    {
                    if (ch == '\\' && inDoubleQuote && !escapedInDoubleQuote)
                        {
                        escapedInDoubleQuote = true;
                        noInlineComment.push_back(ch);
                        continue;
                        }

                    if (ch == '\'' && !inDoubleQuote)
                        {
                        inSingleQuote = !inSingleQuote;
                        }
                    else if (ch == '"' && !inSingleQuote && !escapedInDoubleQuote)
                        {
                        inDoubleQuote = !inDoubleQuote;
                        }
                    else if (ch == '#' && !inSingleQuote && !inDoubleQuote)
                        {
                        break;
                        }
                    noInlineComment.push_back(ch);
                    escapedInDoubleQuote = false;
                    }

                const std::string stripped = trim(noInlineComment);
                if (stripped.empty() || stripped.front() == '#')
                    {
                    continue;
                    }

                const std::size_t colon = stripped.find(':');
                if (colon == std::string::npos)
                    {
                    continue;
                    }

                const std::string key = trim(stripped.substr(0, colon));
                std::string value = trim(stripped.substr(colon + 1));
                if (value.empty())
                    {
                    scalars[key] = "";
                    continue;
                    }

                if (value.front() == '[' && value.back() == ']')
                    {
                    std::string inner = trim(value.substr(1, value.size() - 2));
                    std::vector<std::string> items;
                    std::istringstream itemStream(inner);
                    std::string item;
                    while (std::getline(itemStream, item, ','))
                        {
                        const std::string trimmed = trim(item);
                        if (trimmed.empty())
                            {
                            continue;
                            }
                        items.push_back(unquote(trimmed));
                        }
                    arrays[key] = items;
                    }
                else
                    {
                    scalars[key] = unquote(value);
                    }
                }

            auto getBool = [&](const std::string& key, bool& target)
                {
                auto it = scalars.find(key);
                if (it == scalars.end())
                    {
                    return;
                    }
                const std::string lowered = core::toLowerCopy(it->second);
                if (lowered == "true" || lowered == "1" || lowered == "yes")
                    {
                    target = true;
                    }
                else if (lowered == "false" || lowered == "0" || lowered == "no")
                    {
                    target = false;
                    }
                };
            auto getString = [&](const std::string& key, std::string& target)
                {
                auto it = scalars.find(key);
                if (it != scalars.end())
                    {
                    target = it->second;
                    }
                };
            auto getPath = [&](const std::string& key, std::filesystem::path& target)
                {
                auto it = scalars.find(key);
                if (it != scalars.end())
                    {
                    target = it->second;
                    }
                };
            auto getInt = [&](const std::string& key, int& target)
                {
                auto it = scalars.find(key);
                if (it == scalars.end())
                    {
                    return;
                    }
                try
                    {
                    target = std::stoi(it->second);
                    }
                catch (...)
                    {
                    }
                };
            auto getUInt64 = [&](const std::string& key, std::uintmax_t& target)
                {
                auto it = scalars.find(key);
                if (it == scalars.end())
                    {
                    return;
                    }
                try
                    {
                    target = static_cast<std::uintmax_t>(std::stoull(it->second));
                    }
                catch (...)
                    {
                    }
                };
            auto getStringArray = [&](const std::string& key, std::vector<std::string>& target)
                {
                auto it = arrays.find(key);
                if (it != arrays.end())
                    {
                    target = it->second;
                    }
                };

            parseConfigCommon(getBool, getString, getPath, getInt, getUInt64, getStringArray);
            return { core::ExitCode::success, "" };
            }

        json::JsonValue root;
        core::RunResult parseRes = json::parse(buffer.str(), root);
        if (parseRes.code != core::ExitCode::success)
            {
            return parseRes;
            }

        if (!root.isObject())
            {
            return { core::ExitCode::invalid_usage, "Config file must be a JSON object." };
            }

        const auto& obj = root.getObject();
        auto getBool = [&](const std::string& key, bool& target)
            {
            if (obj.count(key) && obj.at(key).isBool())
                {
                target = obj.at(key).getBool();
                }
            };
        auto getString = [&](const std::string& key, std::string& target)
            {
            if (obj.count(key) && obj.at(key).isString())
                {
                target = obj.at(key).getString();
                }
            };
        auto getPath = [&](const std::string& key, std::filesystem::path& target)
            {
            if (obj.count(key) && obj.at(key).isString())
                {
                target = obj.at(key).getString();
                }
            };
        auto getInt = [&](const std::string& key, int& target)
            {
            if (obj.count(key) && obj.at(key).isNumber())
                {
                target = static_cast<int>(obj.at(key).getNumber());
                }
            };
        auto getUInt64 = [&](const std::string& key, std::uintmax_t& target)
            {
            if (obj.count(key) && obj.at(key).isNumber())
                {
                target = static_cast<std::uintmax_t>(obj.at(key).getNumber());
                }
            };
        auto getStringArray = [&](const std::string& key, std::vector<std::string>& target)
            {
            if (obj.count(key) && obj.at(key).isArray())
                {
                target.clear();
                for (const auto& value : obj.at(key).getArray())
                    {
                    if (value.isString())
                        {
                        target.push_back(value.getString());
                        }
                    }
                }
            };

        parseConfigCommon(getBool, getString, getPath, getInt, getUInt64, getStringArray);
        return { core::ExitCode::success, "" };
        }
    }
