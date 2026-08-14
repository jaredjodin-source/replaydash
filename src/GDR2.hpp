#pragma once

#include "Macro.hpp"

#include <filesystem>
#include <optional>
#include <vector>

namespace replaydash {

class GDR2 {
public:
    static std::optional<Macro> load(
        std::filesystem::path const& path
    );

private:
    static bool readVarInt(
        std::vector<uint8_t> const& data,
        size_t& position,
        uint64_t& value
    );

    static bool readString(
        std::vector<uint8_t> const& data,
        size_t& position,
        std::string& value
    );

    static bool readBool(
        std::vector<uint8_t> const& data,
        size_t& position,
        bool& value
    );

    static bool skip(
        std::vector<uint8_t> const& data,
        size_t& position,
        uint64_t count
    );
};

}
