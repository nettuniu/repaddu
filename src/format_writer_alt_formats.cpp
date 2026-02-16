#include "format_writer_alt_formats.h"

#include "repaddu/logger.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace repaddu::format::detail
    {
    namespace
        {
        std::string escapeJsonString(const std::string& value)
            {
            std::ostringstream ss;
            ss << '"';
            for (char c : value)
                {
                switch (c)
                    {
                    case '"': ss << "\\\""; break;
                    case '\\': ss << "\\\\"; break;
                    case '\b': ss << "\\b"; break;
                    case '\f': ss << "\\f"; break;
                    case '\n': ss << "\\n"; break;
                    case '\r': ss << "\\r"; break;
                    case '\t': ss << "\\t"; break;
                    default:
                        if ('\x00' <= c && c <= '\x1f')
                            {
                            ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                            }
                        else
                            {
                            ss << c;
                            }
                    }
                }
            ss << '"';
            return ss.str();
            }
        }

    core::RunResult writeJsonlOutput(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::OutputChunk>& chunks,
        security::PiiRedactor* redactor)
        {
        const std::string filename = "dataset.jsonl";
        const std::filesystem::path outPath = options.outputPath / filename;

        if (options.dryRun)
            {
            LogInfo("[Dry Run] Would write JSONL: " + filename);
            return { core::ExitCode::success, "" };
            }

        std::ofstream stream(outPath, std::ios::binary);
        if (!stream)
            {
            return { core::ExitCode::io_failure, "Failed to create JSONL output file." };
            }

        std::vector<bool> visited(files.size(), false);

        for (const auto& chunk : chunks)
            {
            for (std::size_t fileIndex : chunk.fileIndices)
                {
                if (visited[fileIndex])
                    {
                    continue;
                    }
                visited[fileIndex] = true;

                core::RunResult readResult;
                core::FileEntry entry = files[fileIndex];
                const std::string content = readFileContent(entry.absolutePath, readResult, &entry.tokenCount, redactor, entry.relativePath.string());

                if (readResult.code != core::ExitCode::success)
                    {
                    return readResult;
                    }

                stream << "{";
                stream << "\"path\": " << escapeJsonString(entry.relativePath.generic_string()) << ", ";
                stream << "\"class\": " << escapeJsonString(core::fileClassLabel(entry.fileClass)) << ", ";
                stream << "\"bytes\": " << entry.sizeBytes << ", ";
                stream << "\"tokens\": " << entry.tokenCount << ", ";
                stream << "\"content\": " << escapeJsonString(content);
                stream << "}\n";
                }
            }

        return { core::ExitCode::success, "" };
        }

    core::RunResult writeHtmlOutput(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        security::PiiRedactor* redactor)
        {
        const std::string filename = "index.html";
        const std::filesystem::path outPath = options.outputPath / filename;

        if (options.dryRun)
            {
            LogInfo("[Dry Run] Would write HTML: " + filename);
            return { core::ExitCode::success, "" };
            }

        std::ofstream stream(outPath);
        if (!stream)
            {
            return { core::ExitCode::io_failure, "Failed to create HTML output file." };
            }

        stream << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Repaddu Code Report</title>
    <style>
        body { font-family: sans-serif; margin: 0; display: flex; height: 100vh; overflow: hidden; }
        #sidebar { width: 300px; border-right: 1px solid #ccc; overflow-y: auto; background: #f5f5f5; padding: 10px; }
        #content { flex: 1; padding: 20px; overflow-y: auto; background: #fff; }
        .file-item { cursor: pointer; padding: 2px 5px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
        .file-item:hover { background: #e0e0e0; }
        .file-item.active { background: #d0d0ff; font-weight: bold; }
        pre { background: #f8f8f8; padding: 10px; border: 1px solid #ddd; overflow-x: auto; }
    </style>
</head>
<body>
    <div id="sidebar"><h3>Files</h3><div id="file-list"></div></div>
    <div id="content"><h2>Select a file to view content</h2><pre id="code-view"></pre></div>
    <script>
        const files = [
)";

        bool first = true;
        for (const auto& entry : files)
            {
            if (!first)
                {
                stream << ",\\n";
                }
            first = false;

            core::RunResult readResult;
            const std::string content = readFileContent(entry.absolutePath, readResult, nullptr, redactor, entry.relativePath.string());

            stream << "{ \"path\": " << escapeJsonString(entry.relativePath.generic_string())
                   << ", \"content\": " << escapeJsonString(content) << " }";
            }

        stream << R"(
        ];

        const listEl = document.getElementById('file-list');
        const codeEl = document.getElementById('code-view');
        const titleEl = document.querySelector('#content h2');

        files.forEach((file, index) => {
            const div = document.createElement('div');
            div.className = 'file-item';
            div.textContent = file.path;
            div.onclick = () => {
                document.querySelectorAll('.file-item').forEach(el => el.classList.remove('active'));
                div.classList.add('active');
                titleEl.textContent = file.path;
                codeEl.textContent = file.content;
            };
            listEl.appendChild(div);
        });
    </script>
</body>
</html>)";

        return { core::ExitCode::success, "" };
        }
    }
