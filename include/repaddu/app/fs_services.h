#ifndef REPADDU_APP_FS_SERVICES_H
#define REPADDU_APP_FS_SERVICES_H

#include "repaddu/core_types.h"
#include "repaddu/io_traversal.h"

namespace repaddu::app
    {
    class RepositoryTraversalService
        {
        public:
            virtual ~RepositoryTraversalService() = default;

            virtual core::RunResult traverse(const core::CliOptions& options,
                io::TraversalResult& traversal) = 0;
        };

    class DefaultRepositoryTraversalService : public RepositoryTraversalService
        {
        public:
            core::RunResult traverse(const core::CliOptions& options,
                io::TraversalResult& traversal) override;
        };
    }

#endif // REPADDU_APP_FS_SERVICES_H
