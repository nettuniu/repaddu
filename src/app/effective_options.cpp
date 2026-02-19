#include "repaddu/app/effective_options.h"

#include "repaddu/language_profiles.h"

namespace repaddu::app
    {
    core::CliOptions buildEffectiveOptions(const core::CliOptions& requestedOptions,
        const std::vector<core::FileEntry>& files)
        {
        core::CliOptions effectiveOptions = requestedOptions;
        if (effectiveOptions.language.empty() || effectiveOptions.buildSystem.empty())
            {
            const core::DetectionResult detected = core::detectLanguageAndBuildSystem(files);
            if (effectiveOptions.language.empty())
                {
                effectiveOptions.language = detected.languageId;
                }
            if (effectiveOptions.buildSystem.empty())
                {
                effectiveOptions.buildSystem = detected.buildSystemId;
                }
            }
        return effectiveOptions;
        }
    }
