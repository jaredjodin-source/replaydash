#pragma once

#include "Macro.hpp"

#include <filesystem>
#include <optional>

namespace replaydash {

class RDM {
public:
    static bool save(
        Macro const& macro,
        std::filesystem::path const& path
    );

    static std::optional<Macro> load(
        std::filesystem::path const& path
    );
};

}
