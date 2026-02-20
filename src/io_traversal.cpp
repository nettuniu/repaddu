#include "repaddu/io_traversal.h"

#include "repaddu/core_types.h"
#include "repaddu/io_binary.h"
#include "repaddu/language_profiles.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <deque>
#include <condition_variable>
#include <cctype>
#include <iterator>
#include <mutex>
#include <thread>

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

        core::RunResult traverseSingleThread(const core::CliOptions& options,
            const std::filesystem::directory_options& dirOptions,
            const std::vector<std::string>& buildFileNamesLower,
            TraversalResult& outResult)
            {
            try
                {
                std::error_code errorCode;
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

            return { core::ExitCode::success, "" };
            }

        core::RunResult traverseParallel(const core::CliOptions& options,
            const std::filesystem::directory_options& dirOptions,
            const std::vector<std::string>& buildFileNamesLower,
            TraversalResult& outResult)
            {
            std::atomic<bool> hasError(false);
            core::RunResult errorResult{ core::ExitCode::success, "" };
            std::mutex errorMutex;
            std::mutex resultMutex;
            std::mutex queueMutex;
            std::condition_variable queueCv;
            std::deque<std::filesystem::path> pending;
            std::size_t activeWorkers = 0;
            bool done = false;

            pending.push_back(options.inputPath);

            auto setError = [&](const core::RunResult& result)
                {
                if (hasError.exchange(true))
                    {
                    return;
                    }
                {
                    std::lock_guard<std::mutex> lock(errorMutex);
                    errorResult = result;
                }
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    done = true;
                }
                queueCv.notify_all();
                };

            auto pushDirectory = [&](const std::filesystem::path& path)
                {
                std::lock_guard<std::mutex> lock(queueMutex);
                pending.push_back(path);
                queueCv.notify_one();
                };

            auto appendBatch = [&](std::vector<std::filesystem::path>& directories,
                std::vector<core::FileEntry>& files,
                std::vector<std::filesystem::path>& cmakeLists,
                std::vector<std::filesystem::path>& buildFiles)
                {
                std::lock_guard<std::mutex> lock(resultMutex);
                outResult.directories.insert(outResult.directories.end(),
                    std::make_move_iterator(directories.begin()),
                    std::make_move_iterator(directories.end()));
                outResult.files.insert(outResult.files.end(),
                    std::make_move_iterator(files.begin()),
                    std::make_move_iterator(files.end()));
                outResult.cmakeLists.insert(outResult.cmakeLists.end(),
                    std::make_move_iterator(cmakeLists.begin()),
                    std::make_move_iterator(cmakeLists.end()));
                outResult.buildFiles.insert(outResult.buildFiles.end(),
                    std::make_move_iterator(buildFiles.begin()),
                    std::make_move_iterator(buildFiles.end()));
                };

            auto processDirectory = [&](const std::filesystem::path& directory)
                {
                std::vector<std::filesystem::path> localDirectories;
                std::vector<core::FileEntry> localFiles;
                std::vector<std::filesystem::path> localCmakeLists;
                std::vector<std::filesystem::path> localBuildFiles;

                std::error_code iterError;
                std::filesystem::directory_iterator iterator(directory, dirOptions, iterError);
                if (iterError)
                    {
                    setError({ core::ExitCode::traversal_failure, "Filesystem traversal failed." });
                    return;
                    }
                const std::filesystem::directory_iterator end;
                for (; iterator != end; iterator.increment(iterError))
                    {
                    if (iterError)
                        {
                        setError({ core::ExitCode::traversal_failure, "Filesystem traversal failed." });
                        return;
                        }
                    if (hasError.load())
                        {
                        return;
                        }

                    const std::filesystem::directory_entry& entry = *iterator;
                    const std::filesystem::path currentPath = entry.path();
                    std::error_code errorCode;
                    std::filesystem::path relativePath = std::filesystem::relative(currentPath, options.inputPath, errorCode);
                    if (errorCode)
                        {
                        setError({ core::ExitCode::traversal_failure, "Failed to compute relative path." });
                        return;
                        }

                    std::error_code typeError;
                    const bool isDirectory = entry.is_directory(typeError);
                    if (typeError)
                        {
                        setError({ core::ExitCode::traversal_failure, "Filesystem traversal failed." });
                        return;
                        }

                    if (isGitPath(relativePath))
                        {
                        continue;
                        }

                    if (!options.includeHidden && shouldSkipHidden(relativePath))
                        {
                        continue;
                        }

                    if (isDirectory)
                        {
                        localDirectories.push_back(relativePath);
                        std::error_code symlinkError;
                        const bool isSymlink = entry.is_symlink(symlinkError);
                        if (symlinkError)
                            {
                            setError({ core::ExitCode::traversal_failure, "Filesystem traversal failed." });
                            return;
                            }
                        if (!options.followSymlinks && isSymlink)
                            {
                            continue;
                            }
                        pushDirectory(currentPath);
                        continue;
                        }

                    std::error_code fileTypeError;
                    const bool isRegular = entry.is_regular_file(fileTypeError);
                    if (fileTypeError)
                        {
                        setError({ core::ExitCode::traversal_failure, "Filesystem traversal failed." });
                        return;
                        }
                    if (!isRegular)
                        {
                        continue;
                        }

                    std::error_code symlinkError;
                    const bool isSymlink = entry.is_symlink(symlinkError);
                    if (symlinkError)
                        {
                        setError({ core::ExitCode::traversal_failure, "Filesystem traversal failed." });
                        return;
                        }
                    if (!options.followSymlinks && isSymlink)
                        {
                        continue;
                        }

                    core::FileEntry fileEntry;
                    fileEntry.absolutePath = currentPath;
                    fileEntry.relativePath = relativePath;
                    fileEntry.sizeBytes = entry.file_size(errorCode);
                    if (errorCode)
                        {
                        setError({ core::ExitCode::io_failure, "Failed to read file size." });
                        return;
                        }
                    fileEntry.extensionLower = core::toLowerCopy(currentPath.extension().string());
                    fileEntry.fileClass = core::classifyExtension(fileEntry.extensionLower);
                    fileEntry.isBinary = looksBinary(currentPath);

                    const std::string filenameLower = normalizeFileName(currentPath.filename().string());
                    if (filenameLower == "cmakelists.txt")
                        {
                        localCmakeLists.push_back(relativePath);
                        }
                    if (std::find(buildFileNamesLower.begin(), buildFileNamesLower.end(), filenameLower)
                        != buildFileNamesLower.end())
                        {
                        localBuildFiles.push_back(relativePath);
                        }
                    localFiles.push_back(std::move(fileEntry));
                    }
                if (hasError.load())
                    {
                    return;
                    }
                appendBatch(localDirectories, localFiles, localCmakeLists, localBuildFiles);
                };

            const unsigned int hardwareThreads = std::thread::hardware_concurrency();
            const std::size_t threadCount = std::max<std::size_t>(1, hardwareThreads == 0 ? 1 : hardwareThreads);
            std::vector<std::thread> workers;
            workers.reserve(threadCount);

            for (std::size_t index = 0; index < threadCount; ++index)
                {
                workers.emplace_back([&]()
                    {
                    while (true)
                        {
                        std::filesystem::path directory;
                        {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        queueCv.wait(lock, [&]() { return done || !pending.empty(); });
                        if (done && pending.empty())
                            {
                            return;
                            }
                        directory = pending.front();
                        pending.pop_front();
                        ++activeWorkers;
                        }

                        processDirectory(directory);

                        {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        if (activeWorkers > 0)
                            {
                            --activeWorkers;
                            }
                        if (!done && pending.empty() && activeWorkers == 0)
                            {
                            done = true;
                            queueCv.notify_all();
                            }
                        }
                        }
                    });
                }

            for (auto& worker : workers)
                {
                worker.join();
                }

            if (hasError.load())
                {
                std::lock_guard<std::mutex> lock(errorMutex);
                return errorResult;
                }

            return { core::ExitCode::success, "" };
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

        core::RunResult traversalResult;
        if (options.parallelTraversal)
            {
            traversalResult = traverseParallel(options, dirOptions, buildFileNamesLower, outResult);
            }
        else
            {
            traversalResult = traverseSingleThread(options, dirOptions, buildFileNamesLower, outResult);
            }
        if (traversalResult.code != core::ExitCode::success)
            {
            return traversalResult;
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
