#include "repaddu/cli_parse.h"

#include <sstream>

namespace repaddu::cli
    {
    std::string helpText()
        {
        std::ostringstream out;
        out << "repaddu - convert a repository/folder into numbered Markdown outputs\n\n";
        out << "Usage:\n";
        out << "  repaddu [options] --input <path> --output <path>\n\n";
        out << "Options:\n";
        out << "  -i, --input <path>          Input repository/folder path.\n";
        out << "  -o, --output <path>         Output directory.\n";
        out << "  --max-files <count>         Maximum number of output files. Default: 0.\n";
        out << "  --max-bytes <bytes>         Maximum bytes per output file. Default: 0.\n";
        out << "  --number-width <n>          Width of numeric prefix. Default: 3.\n";
        out << "  --include-headers           Include only headers (.h/.hpp/.hh/.hxx).\n";
        out << "  --include-sources           Include sources (.c/.cc/.cpp/.cxx).\n";
        out << "  --extensions <csv>          Override include list with explicit extensions.\n";
        out << "  --exclude-extensions <csv>  Exclude these extensions after includes.\n";
        out << "  --include-hidden            Include hidden files/directories.\n";
        out << "  --follow-symlinks           Follow directory symlinks.\n";
        out << "  --single-thread             Force single-threaded traversal.\n";
        out << "  --parallel-traversal        Enable parallel traversal (default).\n";
        out << "  --include-binaries          Include binary files.\n";
        out << "  --max-file-size <bytes>     Skip files larger than this (default 1MB).\n";
        out << "  --force-large               Include large files despite size check.\n";
        out << "  --redact-pii                Redact emails, IPs, and secrets from output.\n";
        out << "  --analyze-only              Scan and report statistics without generating files.\n";
        out << "  --analysis                  Enable symbol analysis (AST/LSP) when available.\n";
        out << "  --analysis-views <csv>      Comma-separated analysis views to emit.\n";
        out << "  --analysis-deep             Enable deeper relationship analysis (optional edges).\n";
        out << "  --analysis-collapse <mode>  none|folder|target. Default: none.\n";
        out << "  --extract-tags              Extract TODO/FIXME-like tags in analyze output.\n";
        out << "  --tag-patterns <path>       Load additional tag patterns from file (one per line).\n";
        out << "  --isolate-docs              Group all documentation files (*.md, *.txt) into a separate chunk.\n";
        out << "  --dry-run                   Simulate execution without writing files.\n";
        out << "  --init                      Generate a default config file (JSON or YAML by --config extension).\n";
        out << "  --config <path>             Config path to load and/or generate. Default: .repaddu.json (auto-load also checks .repaddu.yaml/.repaddu.yml).\n";
        out << "  --format <fmt>              markdown|jsonl|html. Default: markdown.\n";
        out << "  --group-by <mode>           directory|component|type|size. Default: directory.\n";
        out << "  --group-depth <n>           Depth for directory grouping. Default: 1.\n";
        out << "  --component-map <path>      JSON component mapping file for component grouping.\n";
        out << "  --headers-first             Order headers before sources in groups.\n";
        out << "  --emit-tree                 Emit recursive tree listing.\n";
        out << "  --emit-cmake                Emit aggregated CMakeLists.txt output.\n";
        out << "  --emit-build-files          Emit aggregated build-system files.\n";
        out << "  --no-links                  Disable markdown links in overview table of contents.\n";
        out << "  --markers <mode>            fenced|sentinel. Default: fenced.\n";
        out << "  --frontmatter               Add YAML frontmatter metadata before each file content block.\n";
        out << "  --scan-languages            Scan repository and report language percentages only.\n";
        out << "  --language <id>             auto|c|cpp|rust|python. Default: auto.\n";
        out << "  --build-system <id>         auto|cmake|make|meson|bazel|cargo|npm|python. Default: auto.\n";
        out << "  -h, --help                  Show help.\n";
        out << "  --version                   Show version.\n";
        return out.str();
        }

    std::string versionText()
        {
        return "repaddu 0.1.0\n";
        }
    }
