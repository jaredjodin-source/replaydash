#include "GDR2.hpp"

#include <fstream>
#include <cstring>
#include <cstdint>

namespace replaydash {

static MacroButton convertButton(uint8_t button) {
    switch (button) {
        case 2:
            return MacroButton::Left;

        case 3:
            return MacroButton::Right;

        default:
            return MacroButton::Jump;
    }
}

static bool readFile(
    std::filesystem::path const& path,
    std::vector<uint8_t>& data
) {
    std::ifstream file(path, std::ios::binary);

    if (!file)
        return false;

    file.seekg(0, std::ios::end);

    auto size = static_cast<size_t>(file.tellg());

    if (size == 0)
        return false;

    file.seekg(0, std::ios::beg);

    data.resize(size);

    file.read(
        reinterpret_cast<char*>(data.data()),
        static_cast<std::streamsize>(size)
    );

    return file.good();
}

std::optional<Macro> GDR2::load(
    std::filesystem::path const& path
) {
    std::vector<uint8_t> data;

    if (!readFile(path, data))
        return std::nullopt;

    if (data.size() < 4)
        return std::nullopt;

    if (
        data[0] != 'G' ||
        data[1] != 'D' ||
        data[2] != 'R'
    ) {
        return std::nullopt;
    }

    size_t position = 3;

    uint64_t version = 0;

    if (!readVarInt(data, position, version))
        return std::nullopt;

    if (version != 2)
        return std::nullopt;

    std::string extension;

    if (!readString(data, position, extension))
        return std::nullopt;

    std::string author;

    if (!readString(data, position, author))
        return std::nullopt;

    std::string description;

    if (!readString(data, position, description))
        return std::nullopt;

    if (position + 4 > data.size())
        return std::nullopt;

    position += 4;

    uint64_t gameVersion = 0;

    if (!readVarInt(data, position, gameVersion))
        return std::nullopt;

    if (position + sizeof(double) > data.size())
        return std::nullopt;

    double fps = 240.0;

    std::memcpy(
        &fps,
        data.data() + position,
        sizeof(double)
    );

    position += sizeof(double);

    uint64_t seed = 0;
    uint64_t coins = 0;

    if (!readVarInt(data, position, seed))
        return std::nullopt;

    if (!readVarInt(data, position, coins))
        return std::nullopt;

    bool ldm = false;
    bool platformer = false;

    if (!readBool(data, position, ldm))
        return std::nullopt;

    if (!readBool(data, position, platformer))
        return std::nullopt;

    std::string botName;

    if (!readString(data, position, botName))
        return std::nullopt;

    uint64_t botVersion = 0;

    if (!readVarInt(data, position, botVersion))
        return std::nullopt;

    uint64_t levelID = 0;

    if (!readVarInt(data, position, levelID))
        return std::nullopt;

    std::string levelName;

    if (!readString(data, position, levelName))
        return std::nullopt;

    uint64_t extensionSize = 0;

    if (!readVarInt(data, position, extensionSize))
        return std::nullopt;

    if (!skip(data, position, extensionSize))
        return std::nullopt;

    uint64_t deathCount = 0;

    if (!readVarInt(data, position, deathCount))
        return std::nullopt;

    for (uint64_t i = 0; i < deathCount; ++i) {
        uint64_t deathDelta = 0;

        if (!readVarInt(data, position, deathDelta))
            return std::nullopt;
    }

    uint64_t totalInputs = 0;
    uint64_t p1Inputs = 0;

    if (!readVarInt(data, position, totalInputs))
        return std::nullopt;

    if (!readVarInt(data, position, p1Inputs))
        return std::nullopt;

    if (p1Inputs > totalInputs)
        return std::nullopt;

    Macro macro;

    macro.name = "Imported GDR2";

    if (fps > 0.0 && fps < 10000.0)
        macro.fps = fps;

    uint64_t frame = 0;

    for (uint64_t i = 0; i < p1Inputs; ++i) {
        uint64_t packed = 0;

        if (!readVarInt(data, position, packed))
            return std::nullopt;

        uint64_t delta = 0;
        uint8_t button = 1;
        bool down = false;

        if (platformer) {
            delta = packed >> 3;

            button = static_cast<uint8_t>(
                (packed >> 1) & 3
            );

            down = (packed & 1) != 0;
        }
        else {
            delta = packed >> 1;
            down = (packed & 1) != 0;
        }

        frame += delta;

        MacroEvent event;

        event.frame = frame;
        event.button = convertButton(button);
        event.down = down;
        event.player2 = false;

        macro.events.push_back(event);

        if (!extension.empty()) {
            uint64_t physicsSize = 0;

            if (!readVarInt(
                data,
                position,
                physicsSize
            )) {
                return std::nullopt;
            }

            if (!skip(
                data,
                position,
                physicsSize
            )) {
                return std::nullopt;
            }
        }
    }

    frame = 0;

    uint64_t p2Inputs =
        totalInputs - p1Inputs;

    for (uint64_t i = 0; i < p2Inputs; ++i) {
        uint64_t packed = 0;

        if (!readVarInt(data, position, packed))
            return std::nullopt;

        uint64_t delta = 0;
        uint8_t button = 1;
        bool down = false;

        if (platformer) {
            delta = packed >> 3;

            button = static_cast<uint8_t>(
                (packed >> 1) & 3
            );

            down = (packed & 1) != 0;
        }
        else {
            delta = packed >> 1;
            down = (packed & 1) != 0;
        }

        frame += delta;

        MacroEvent event;

        event.frame = frame;
        event.button = convertButton(button);
        event.down = down;
        event.player2 = true;

        macro.events.push_back(event);

        if (!extension.empty()) {
            uint64_t physicsSize = 0;

            if (!readVarInt(
                data,
                position,
                physicsSize
            )) {
                return std::nullopt;
            }

            if (!skip(
                data,
                position,
                physicsSize
            )) {
                return std::nullopt;
            }
        }
    }

    if (macro.events.empty())
        return std::nullopt;

    return macro;
}

bool GDR2::readVarInt(
    std::vector<uint8_t> const& data,
    size_t& position,
    uint64_t& value
) {
    value = 0;

    int shift = 0;

    while (position < data.size()) {
        uint8_t byte = data[position++];

        value |=
            static_cast<uint64_t>(byte & 0x7F)
            << shift;

        if ((byte & 0x80) == 0)
            return true;

        shift += 7;

        if (shift >= 64)
            return false;
    }

    return false;
}

bool GDR2::readString(
    std::vector<uint8_t> const& data,
    size_t& position,
    std::string& value
) {
    uint64_t size = 0;

    if (!readVarInt(data, position, size))
        return false;

    if (size > data.size() - position)
        return false;

    value.assign(
        reinterpret_cast<char const*>(
            data.data() + position
        ),
        static_cast<size_t>(size)
    );

    position += static_cast<size_t>(size);

    return true;
}

bool GDR2::readBool(
    std::vector<uint8_t> const& data,
    size_t& position,
    bool& value
) {
    if (position >= data.size())
        return false;

    value = data[position++] != 0;

    return true;
}

bool GDR2::skip(
    std::vector<uint8_t> const& data,
    size_t& position,
    uint64_t count
) {
    if (count > data.size() - position)
        return false;

    position += static_cast<size_t>(count);

    return true;
}

}
