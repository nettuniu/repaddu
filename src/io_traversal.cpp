#include "repaddu/io_traversal.h"

#include "repaddu/core_types.h"
#include "repaddu/io_binary.h"
#include "repaddu/language_profiles.h"

#include <algorithm>
#include <filesystem>
#include <cctype>

namespace repaddu::io
    {
    namespace
        {
        bool isHiddenComponent(const std::filesystem::path& component)
            {
            const std::string name = component.string();
            return !name.empty() && name.front() == '.';
            }

        bool shouldSkipHidden(const std::filesystem::path& relativePath)
            {
            for (const auto& component : relativePath)
                {
                if (isHiddenComponent(component))
                    {
                    return true;
                    }
                }
            return false;
            }

        bool isGitPath(const std::filesystem::path& relativePath)
            {
            for (const auto& component : relativePath)
                {
                if (component == ".git")
                    {
                    return true;
                    }
                }
            return false;
            }

        std::string normalizeFileName(std::string value)
            {
            for (char& ch : value)
                {
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                }
            return value;
            }
        }

    core::RunResult traverseRepository(const core::CliOptions& options, TraversalResult& outResult)
        {
        outResult = TraversalResult{};

        const std::vector<std::string> buildFileNames = core::resolveBuildFileNames(options);
        std::vector<std::string> buildFileNamesLower;
        buildFileNamesLower.reserve(buildFileNames.size());
        for (const std::string& name : buildFileNames)
            {
            buildFileNamesLower.push_back(normalizeFileName(name));
            }

        std::error_code errorCode;
        if (!std::filesystem::exists(options.inputPath, errorCode))
            {
            return { core::ExitCode::io_failure, "Input path does not exist." };
            }

        std::filesystem::directory_options dirOptions = std::filesystem::directory_options::skip_permission_denied;
        if (options.followSymlinks)
            {
            dirOptions |= std::filesystem::directory_options::follow_directory_symlink;
            }

        try
            {
            std::filesystem::recursive_directory_iterator iterator(options.inputPath, dirOptions);
            const std::filesystem::recursive_directory_iterator end;

            for (; iterator != end; ++iterator)
                {
                const std::filesystem::directory_entry& entry = *iterator;
                const std::filesystem::path currentPath = entry.path();
                std::filesystem::path relativePath = std::filesystem::relative(currentPath, options.inputPath, errorCode);
                if (errorCode)
                    {
                    return { core::ExitCode::traversal_failure, "Failed to compute relative path." };
                    }

                if (isGitPath(relativePath))
                    {
                    if (entry.is_directory())
                        {
                        iterator.disable_recursion_pending();
                        }
                    continue;
                    }

                if (!options.includeHidden && shouldSkipHidden(relativePath))
                    {
                    if (entry.is_directory())
                        {
                        iterator.disable_recursion_pending();
                        }
                    continue;
                    }

                if (entry.is_directory())
                    {
                    outResult.directories.push_back(relativePath);
                    continue;
                    }

                if (!entry.is_regular_file())
                    {
                    continue;
                    }

                if (!options.followSymlinks && entry.is_symlink())
                    {
                    continue;
                    }

                const std::string filenameLower = normalizeFileName(entry.path().filename().string());
                if (filenameLower == "cmakelists.txt")
                    {
                    outResult.cmakeLists.push_back(relativePath);
                    }
                if (std::find(buildFileNamesLower.begin(), buildFileNamesLower.end(), filenameLower)
                    != buildFileNamesLower.end())
                    {
                    outResult.buildFiles.push_back(relativePath);
                    }

                core::FileEntry fileEntry;
                fileEntry.absolutePath = currentPath;
                fileEntry.relativePath = relativePath;
                fileEntry.sizeBytes = entry.file_size(errorCode);
                if (errorCode)
                    {
                    return { core::ExitCode::io_failure, "Failed to read file size." };
                    }
                fileEntry.extensionLower = core::toLowerCopy(currentPath.extension().string());
                fileEntry.fileClass = core::classifyExtension(fileEntry.extensionLower);
                fileEntry.isBinary = looksBinary(currentPath);

                outResult.files.push_back(std::move(fileEntry));
                }
            }
        catch (const std::filesystem::filesystem_error&)
            {
            return { core::ExitCode::traversal_failure, "Filesystem traversal failed." };
            }

        std::sort(outResult.files.begin(), outResult.files.end(),
            [](const core::FileEntry& a, const core::FileEntry& b)
            {
            return a.relativePath.string() < b.relativePath.string();
            });

        std::sort(outResult.directories.begin(), outResult.directories.end(),
            [](const std::filesystem::path& a, const std::filesystem::path& b)
            {
            return a.string() < b.string();
            });

        std::sort(outResult.cmakeLists.begin(), outResult.cmakeLists.end(),
            [](const std::filesystem::path& a, const std::filesystem::path& b)
            {
            return a.string() < b.string();
            });

        std::sort(outResult.buildFiles.begin(), outResult.buildFiles.end(),
            [](const std::filesystem::path& a, const std::filesystem::path& b)
            {
            return a.string() < b.string();
            });

        return { core::ExitCode::success, "" };
        }
    }
