#include "repaddu/entrypoint_main.h"

#include <vector>

int main(int argc, char** argv)
    {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i)
        {
        args.emplace_back(argv[i]);
        }

    return repaddu::entrypoint::runMain(args);
    }
