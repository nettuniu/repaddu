#include "repaddu/app/fs_services.h"

#include "repaddu/io_traversal.h"

namespace repaddu::app
    {
    core::RunResult DefaultRepositoryTraversalService::traverse(const core::CliOptions& options,
        io::TraversalResult& traversal)
        {
        return io::traverseRepository(options, traversal);
        }
    }
