#ifndef REPADDU_CLI_BOOTSTRAP_H
#define REPADDU_CLI_BOOTSTRAP_H

#include "repaddu/cli_parse.h"

#include <vector>

namespace repaddu::cli
    {
    ParseResult parseArgsWithConfig(const std::vector<std::string>& args);
    }

#endif // REPADDU_CLI_BOOTSTRAP_H
