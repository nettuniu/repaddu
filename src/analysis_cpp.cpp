#include "repaddu/analysis_cpp.h"

#include "repaddu/analysis_graph.h"
#include "repaddu/core_types.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclCXX.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <memory>

namespace repaddu::analysis
    {
    namespace
        {
        std::filesystem::path resolveCompilationDatabasePath(const std::filesystem::path& input)
            {
            if (input.empty())
                {
                return {};
                }
            if (std::filesystem::is_regular_file(input))
                {
                return input.parent_path();
                }
            return input;
            }

        class SymbolCollector : public clang::ast_matchers::MatchFinder::MatchCallback
            {
            public:
                SymbolCollector(AnalysisGraph& graph, bool deep) : graph_(graph), deep_(deep) {}

                void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
                    {
                    if (const auto* record = result.Nodes.getNodeAs<clang::CXXRecordDecl>("classDecl"))
                        {
                        if (record->isImplicit())
                            {
                            return;
                            }
                        if (!record->getIdentifier())
                            {
                            return;
                            }
                        if (!isPublic(record->getAccess()))
                            {
                            return;
                            }

                        SymbolNodeInput input;
                        input.kind = SymbolKind::class_;
                        input.name = record->getNameAsString();
                        input.qualifiedName = record->getQualifiedNameAsString();
                        input.containerName = contextQualifiedName(record->getDeclContext());
                        input.isPublic = true;
                        if (const auto* sm = result.SourceManager)
                            {
                            input.sourcePath = sm->getFilename(record->getLocation()).str();
                            }
                        graph_.addSymbol(input);

                        for (const auto& base : record->bases())
                            {
                            if (!isPublic(base.getAccessSpecifier()))
                                {
                                continue;
                                }
                            const auto* baseDecl = base.getType()->getAsCXXRecordDecl();
                            if (!baseDecl || !baseDecl->getIdentifier())
                                {
                                continue;
                                }

                            SymbolNodeInput baseInput;
                            baseInput.kind = SymbolKind::class_;
                            baseInput.name = baseDecl->getNameAsString();
                            baseInput.qualifiedName = baseDecl->getQualifiedNameAsString();
                            baseInput.containerName = contextQualifiedName(baseDecl->getDeclContext());
                            baseInput.isPublic = true;
                            const SymbolId baseId = graph_.addSymbol(baseInput);
                            const SymbolId derivedId = graph_.addSymbol(input);
                            graph_.addEdge(derivedId, baseId, EdgeKind::inherits);
                            }
                        }

                    if (const auto* method = result.Nodes.getNodeAs<clang::CXXMethodDecl>("methodDecl"))
                        {
                        if (method->isImplicit())
                            {
                            return;
                            }
                        if (!method->getIdentifier())
                            {
                            return;
                            }
                        if (!isPublic(method->getAccess()))
                            {
                            return;
                            }

                        SymbolNodeInput input;
                        input.kind = SymbolKind::method_;
                        input.name = method->getNameAsString();
                        input.qualifiedName = method->getQualifiedNameAsString();
                        if (const auto* parent = method->getParent())
                            {
                            input.containerName = parent->getQualifiedNameAsString();
                            }
                        input.isPublic = true;
                        if (const auto* sm = result.SourceManager)
                            {
                            input.sourcePath = sm->getFilename(method->getLocation()).str();
                            }
                        graph_.addSymbol(input);

                        if (!deep_)
                            {
                            return;
                            }

                        for (const auto* overridden : method->overridden_methods())
                            {
                            if (!overridden || overridden->isImplicit())
                                {
                                continue;
                                }
                            if (!overridden->getIdentifier())
                                {
                                continue;
                                }
                            if (!isPublic(overridden->getAccess()))
                                {
                                continue;
                                }

                            SymbolNodeInput overriddenInput;
                            overriddenInput.kind = SymbolKind::method_;
                            overriddenInput.name = overridden->getNameAsString();
                            overriddenInput.qualifiedName = overridden->getQualifiedNameAsString();
                            if (const auto* parent = overridden->getParent())
                                {
                                overriddenInput.containerName = parent->getQualifiedNameAsString();
                                }
                            overriddenInput.isPublic = true;
                            const SymbolId baseMethodId = graph_.addSymbol(overriddenInput);
                            const SymbolId methodId = graph_.addSymbol(input);
                            graph_.addEdge(methodId, baseMethodId, EdgeKind::overrides);
                            if (overridden->isPureVirtual())
                                {
                                graph_.addEdge(baseMethodId, methodId, EdgeKind::implemented_by);
                                }
                            }
                        }
                    }

            private:
                static std::string contextQualifiedName(const clang::DeclContext* context)
                    {
                    if (!context)
                        {
                        return {};
                        }
                    if (const auto* named = clang::dyn_cast<clang::NamedDecl>(context))
                        {
                        return named->getQualifiedNameAsString();
                        }
                    return {};
                    }

                static bool isPublic(clang::AccessSpecifier access)
                    {
                    return access == clang::AS_public || access == clang::AS_none;
                    }

                AnalysisGraph& graph_;
                bool deep_ = false;
            };
        }

    core::RunResult analyzeCppProject(const CppAnalysisOptions& options, AnalysisGraph& graph)
        {
        const std::filesystem::path compDbPath = resolveCompilationDatabasePath(options.compileCommandsPath);
        if (compDbPath.empty())
            {
            return { core::ExitCode::invalid_usage, "compile_commands.json path is required." };
            }

        std::string error;
        std::unique_ptr<clang::tooling::CompilationDatabase> database =
            clang::tooling::CompilationDatabase::loadFromDirectory(compDbPath.string(), error);
        if (!database)
            {
            return { core::ExitCode::io_failure, "Failed to load compilation database: " + error };
            }

        std::vector<std::string> files = database->getAllFiles();
        if (files.empty())
            {
            return { core::ExitCode::invalid_usage, "Compilation database contains no files." };
            }

        clang::tooling::ClangTool tool(*database, files);

        SymbolCollector collector(graph, options.deep);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(clang::ast_matchers::cxxRecordDecl(clang::ast_matchers::isDefinition()).bind("classDecl"),
            &collector);
        finder.addMatcher(clang::ast_matchers::cxxMethodDecl(clang::ast_matchers::isDefinition()).bind("methodDecl"),
            &collector);

        const int result = tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
        if (result != 0)
            {
            return { core::ExitCode::io_failure, "Clang tool execution failed." };
            }

        return { core::ExitCode::success, "" };
        }
    }
