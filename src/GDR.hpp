#pragma once

#include "Macro.hpp"

#include <filesystem>
#include <optional>

namespace replaydash {

class GDR {
public:
    static std::optional<Macro> load(
        std::filesystem::path const& path
    );
};

}
