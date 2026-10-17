#include "repaddu/format_manifest.h"

#include <iomanip>
#include <sstream>

namespace repaddu::format
    {
    namespace
        {
        std::string escapeJson(std::string_view value)
            {
            std::string out;
            out.reserve(value.size() + 8);
            for (char ch : value)
                {
                switch (ch)
                    {
                    case '\"': out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\b': out += "\\b"; break;
                    case '\f': out += "\\f"; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default:
                        if (static_cast<unsigned char>(ch) < 0x20)
                            {
                            std::ostringstream hex;
                            hex << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                                << static_cast<int>(static_cast<unsigned char>(ch));
                            out += hex.str();
                            }
                        else
                            {
                            out += ch;
                            }
                        break;
                    }
                }
            return out;
            }
        }

    std::string buildManifestJson(const ContextManifestData& data)
        {
        std::ostringstream json;
        json << "{\n";
        json << "  \"schema_version\": " << data.schemaVersion << ",\n";
        json << "  \"generator\": {\n";
        json << "    \"name\": \"" << escapeJson(data.generatorName) << "\",\n";
        json << "    \"version\": \"" << escapeJson(data.generatorVersion) << "\"\n";
        json << "  },\n";
        json << "  \"repository\": {\n";
        json << "    \"root\": \"" << escapeJson(data.repositoryRoot) << "\"";
        if (!data.gitCommit.empty())
            {
            json << ",\n    \"git_commit\": \"" << escapeJson(data.gitCommit) << "\"";
            }
        if (!data.gitBranch.empty())
            {
            json << ",\n    \"git_branch\": \"" << escapeJson(data.gitBranch) << "\"";
            }
        json << "\n  },\n";

        json << "  \"stats\": {\n";
        json << "    \"total_files\": " << data.stats.totalFiles << ",\n";
        json << "    \"total_bytes\": " << data.stats.totalBytes << ",\n";
        json << "    \"total_tokens\": " << data.stats.totalTokens << ",\n";
        json << "    \"total_outputs\": " << data.stats.totalOutputs << "\n";
        json << "  },\n";

        json << "  \"files\": [\n";
        for (std::size_t i = 0; i < data.files.size(); ++i)
            {
            const auto& file = data.files[i];
            json << "    {\n";
            json << "      \"path\": \"" << escapeJson(file.path) << "\",\n";
            json << "      \"sha256\": \"" << escapeJson(file.sha256) << "\",\n";
            json << "      \"size_bytes\": " << file.sizeBytes << ",\n";
            json << "      \"token_count\": " << file.tokenCount << ",\n";
            json << "      \"language\": \"" << escapeJson(file.language) << "\",\n";
            json << "      \"file_class\": \"" << escapeJson(file.fileClass) << "\",\n";
            json << "      \"output_chunk\": \"" << escapeJson(file.outputChunk) << "\"\n";
            json << "    }" << (i + 1 < data.files.size() ? "," : "") << "\n";
            }
        json << "  ],\n";

        json << "  \"outputs\": [\n";
        for (std::size_t i = 0; i < data.outputs.size(); ++i)
            {
            const auto& out = data.outputs[i];
            json << "    {\n";
            json << "      \"filename\": \"" << escapeJson(out.filename) << "\",\n";
            json << "      \"size_bytes\": " << out.sizeBytes << "\n";
            json << "    }" << (i + 1 < data.outputs.size() ? "," : "") << "\n";
            }
        json << "  ]\n";

        if (data.exclusionReport.totalExcluded() > 0)
            {
            json << "  ,\n  \"exclusions\": {\n";
            json << "    \"total_excluded\": " << data.exclusionReport.totalExcluded() << ",\n";
            json << "    \"by_reason\": {\n";
            json << "      \"binary\": " << data.exclusionReport.countBinary << ",\n";
            json << "      \"too_large\": " << data.exclusionReport.countTooLarge << ",\n";
            json << "      \"vendor\": " << data.exclusionReport.countVendor << ",\n";
            json << "      \"generated\": " << data.exclusionReport.countGenerated << ",\n";
            json << "      \"extension_filter\": " << data.exclusionReport.countExtension << ",\n";
            json << "      \"hidden\": " << data.exclusionReport.countHidden << ",\n";
            json << "      \"noise\": " << data.exclusionReport.countNoise << "\n";
            json << "    },\n";
            json << "    \"files\": [\n";
            for (std::size_t i = 0; i < data.exclusionReport.excludedFiles.size(); ++i)
                {
                const auto& ex = data.exclusionReport.excludedFiles[i];
                json << "      {\n";
                json << "        \"path\": \"" << escapeJson(ex.relativePath.generic_string()) << "\",\n";
                json << "        \"reason\": \"" << escapeJson(exclusionReasonLabel(ex.reason)) << "\",\n";
                json << "        \"size_bytes\": " << ex.sizeBytes << "\n";
                json << "      }" << (i + 1 < data.exclusionReport.excludedFiles.size() ? "," : "") << "\n";
                }
            json << "    ]\n";
            json << "  }\n";
            }

        json << "}\n";

        return json.str();
        }
    }
