#include "repaddu/grouping_strategies.h"

#include "repaddu/core_types.h"
#include "repaddu/language_profiles.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <unordered_map>

namespace repaddu::grouping
    {
    namespace
        {
        std::vector<std::string> normalizeExtensions(const std::vector<std::string>& values)
            {
            std::vector<std::string> result;
            result.reserve(values.size());
            for (const std::string& value : values)
                {
                std::string normalized = core::toLowerCopy(value);
                if (!normalized.empty() && normalized.front() != '.')
                    {
                    normalized.insert(normalized.begin(), '.');
                    }
                result.push_back(std::move(normalized));
                }
            return result;
            }

        std::vector<std::string> extensionsForLanguage(const core::CliOptions& options,
            const core::LanguageProfile& profile)
            {
            std::vector<std::string> result;
            if (profile.supportsHeaders)
                {
                if (options.includeHeaders && !options.includeSources)
                    {
                    result = normalizeExtensions(profile.headerExtensions);
                    }
                else if (options.includeSources && !options.includeHeaders)
                    {
                    result = normalizeExtensions(profile.sourceExtensions);
                    }
                else
                    {
                    result = normalizeExtensions(profile.sourceExtensions);
                    const std::vector<std::string> headers = normalizeExtensions(profile.headerExtensions);
                    result.insert(result.end(), headers.begin(), headers.end());
                    }
                }
            else
                {
                result = normalizeExtensions(profile.sourceExtensions);
                }
            return result;
            }

        bool extensionAllowed(const std::string& extensionLower,
            const std::vector<std::string>& include,
            const std::vector<std::string>& exclude)
            {
            if (!exclude.empty())
                {
                if (std::find(exclude.begin(), exclude.end(), extensionLower) != exclude.end())
                    {
                    return false;
                    }
                }
            if (!include.empty())
                {
                return std::find(include.begin(), include.end(), extensionLower) != include.end();
                }
            return true;
            }

        std::vector<std::size_t> applyHeadersFirst(const std::vector<core::FileEntry>& files,
            const std::vector<std::size_t>& indices,
            bool headersFirst)
            {
            std::vector<std::size_t> result = indices;
            if (headersFirst)
                {
                std::sort(result.begin(), result.end(),
                    [&files](std::size_t lhs, std::size_t rhs)
                    {
                    const core::FileEntry& left = files[lhs];
                    const core::FileEntry& right = files[rhs];
                    if (left.fileClass != right.fileClass)
                        {
                        return static_cast<int>(left.fileClass) < static_cast<int>(right.fileClass);
                        }
                    return left.relativePath.string() < right.relativePath.string();
                    });
                }
            else
                {
                std::sort(result.begin(), result.end(),
                    [&files](std::size_t lhs, std::size_t rhs)
                    {
                    return files[lhs].relativePath.string() < files[rhs].relativePath.string();
                    });
                }
            return result;
            }

        std::string groupKeyForPath(const core::CliOptions& options, const core::FileEntry& entry,
            const ComponentMap* componentMap)
            {
            switch (options.groupBy)
                {
                case core::GroupingMode::directory:
                    {
                    int depth = options.groupDepth;
                    if (depth <= 0)
                        {
                        depth = 1;
                        }
                    std::string key;
                    int currentDepth = 0;
                    for (const auto& component : entry.relativePath)
                        {
                        if (currentDepth >= depth)
                            {
                            break;
                            }
                        if (!key.empty())
                            {
                            key += '/';
                            }
                        key += component.string();
                        ++currentDepth;
                        }
                    if (key.empty())
                        {
                        key = "root";
                        }
                    return key;
                    }
                case core::GroupingMode::component:
                    {
                    if (componentMap != nullptr)
                        {
                        return resolveComponent(*componentMap, entry.relativePath);
                        }
                    return "unmapped";
                    }
                case core::GroupingMode::type:
                    return core::fileClassLabel(entry.fileClass);
                case core::GroupingMode::size:
                    return "size";
                }
            return "group";
            }

        std::vector<core::OutputChunk> sizeBalancedChunks(const core::CliOptions& options,
            const std::vector<core::FileEntry>& files,
            const std::vector<std::size_t>& indices)
            {
            std::vector<std::size_t> sorted = indices;
            std::sort(sorted.begin(), sorted.end(),
                [&files](std::size_t lhs, std::size_t rhs)
                {
                if (files[lhs].sizeBytes != files[rhs].sizeBytes)
                    {
                    return files[lhs].sizeBytes > files[rhs].sizeBytes;
                    }
                return files[lhs].relativePath.string() < files[rhs].relativePath.string();
                });

            std::vector<core::OutputChunk> chunks;
            if (options.maxBytes > 0)
                {
                const std::uintmax_t capacity = options.maxBytes;
                for (std::size_t index : sorted)
                    {
                    bool placed = false;
                    for (auto& chunk : chunks)
                        {
                        std::uintmax_t total = 0;
                        for (std::size_t fileIndex : chunk.fileIndices)
                            {
                            total += files[fileIndex].sizeBytes;
                            }
                        if (total + files[index].sizeBytes <= capacity)
                            {
                            chunk.fileIndices.push_back(index);
                            placed = true;
                            break;
                            }
                        }
                    if (!placed)
                        {
                        core::OutputChunk chunk;
                        chunk.category = "size";
                        chunk.title = "Size-balanced chunk";
                        chunk.fileIndices.push_back(index);
                        chunks.push_back(std::move(chunk));
                        }
                    }
                }
            else if (options.maxFiles > 0)
                {
                const int binCount = options.maxFiles;
                chunks.resize(static_cast<std::size_t>(binCount));
                for (int i = 0; i < binCount; ++i)
                    {
                    chunks[static_cast<std::size_t>(i)].category = "size";
                    chunks[static_cast<std::size_t>(i)].title = "Size-balanced chunk";
                    }

                for (std::size_t index : sorted)
                    {
                    std::size_t best = 0;
                    std::uintmax_t bestSize = std::numeric_limits<std::uintmax_t>::max();
                    for (std::size_t i = 0; i < chunks.size(); ++i)
                        {
                        std::uintmax_t total = 0;
                        for (std::size_t fileIndex : chunks[i].fileIndices)
                            {
                            total += files[fileIndex].sizeBytes;
                            }
                        if (total < bestSize)
                            {
                            bestSize = total;
                            best = i;
                            }
                        }
                    chunks[best].fileIndices.push_back(index);
                    }

                chunks.erase(std::remove_if(chunks.begin(), chunks.end(),
                    [](const core::OutputChunk& chunk)
                    {
                    return chunk.fileIndices.empty();
                    }), chunks.end());
                }
            else
                {
                core::OutputChunk chunk;
                chunk.category = "size";
                chunk.title = "Size-balanced chunk";
                chunk.fileIndices = sorted;
                chunks.push_back(std::move(chunk));
                }

            for (auto& chunk : chunks)
                {
                std::sort(chunk.fileIndices.begin(), chunk.fileIndices.end(),
                    [&files](std::size_t lhs, std::size_t rhs)
                    {
                    return files[lhs].relativePath.string() < files[rhs].relativePath.string();
                    });
                }

            return chunks;
            }
        }

    GroupingResult filterAndGroupFiles(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const ComponentMap* componentMap,
        core::RunResult& outResult)
        {
        outResult = { core::ExitCode::success, "" };
        GroupingResult result;

        std::vector<std::string> includeExt = normalizeExtensions(options.extensions);
        const std::vector<std::string> excludeExt = normalizeExtensions(options.excludeExtensions);
        const core::LanguageProfile* languageProfile = nullptr;

        if (includeExt.empty() && !options.language.empty())
            {
            languageProfile = core::findLanguageProfile(options.language);
            if (languageProfile != nullptr)
                {
                includeExt = extensionsForLanguage(options, *languageProfile);
                }
            }

        for (std::size_t index = 0; index < files.size(); ++index)
            {
            const core::FileEntry& entry = files[index];
            if (!options.includeBinaries && entry.isBinary)
                {
                continue;
                }
            if (!extensionAllowed(entry.extensionLower, includeExt, excludeExt))
                {
                continue;
                }

            if (includeExt.empty())
                {
                if (options.includeHeaders && !options.includeSources)
                    {
                    if (entry.fileClass != core::FileClass::header)
                        {
                        continue;
                        }
                    }
                else if (options.includeSources && !options.includeHeaders)
                    {
                    if (entry.fileClass != core::FileClass::source)
                        {
                        continue;
                        }
                    }
                else
                    {
                    if (entry.fileClass == core::FileClass::other)
                        {
                        continue;
                        }
                    }
                }

            result.includedIndices.push_back(index);
            }

        if (options.groupBy == core::GroupingMode::size)
            {
            core::Group group;
            group.name = "size";
            group.fileIndices = result.includedIndices;
            result.groups.push_back(std::move(group));
            return result;
            }

        std::unordered_map<std::string, std::vector<std::size_t>> buckets;
        for (std::size_t index : result.includedIndices)
            {
            const core::FileEntry& entry = files[index];
            const std::string key = groupKeyForPath(options, entry, componentMap);
            buckets[key].push_back(index);
            }

        result.groups.reserve(buckets.size());
        for (auto& bucket : buckets)
            {
            core::Group group;
            group.name = bucket.first;
            group.fileIndices = applyHeadersFirst(files, bucket.second, options.headersFirst);
            result.groups.push_back(std::move(group));
            }

        std::sort(result.groups.begin(), result.groups.end(),
            [](const core::Group& lhs, const core::Group& rhs)
            {
            return lhs.name < rhs.name;
            });

        if (options.groupBy == core::GroupingMode::type)
            {
            std::stable_sort(result.groups.begin(), result.groups.end(),
                [](const core::Group& lhs, const core::Group& rhs)
                {
                auto rank = [](const std::string& name)
                    {
                    if (name == "header")
                        {
                        return 0;
                        }
                    if (name == "source")
                        {
                        return 1;
                        }
                    return 2;
                    };
                const int leftRank = rank(lhs.name);
                const int rightRank = rank(rhs.name);
                if (leftRank != rightRank)
                    {
                    return leftRank < rightRank;
                    }
                return lhs.name < rhs.name;
                });
            }

        return result;
        }

    std::vector<core::OutputChunk> chunkGroups(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::Group>& groups,
        core::RunResult& outResult)
        {
        outResult = { core::ExitCode::success, "" };

        if (options.groupBy == core::GroupingMode::size)
            {
            if (groups.empty())
                {
                return {};
                }
            return sizeBalancedChunks(options, files, groups.front().fileIndices);
            }

        std::vector<core::OutputChunk> chunks;
        for (const core::Group& group : groups)
            {
            core::OutputChunk chunk;
            chunk.category = core::sanitizeName(group.name);
            chunk.title = group.name;
            chunk.fileIndices = group.fileIndices;
            chunks.push_back(std::move(chunk));
            }

        return chunks;
        }
    }
