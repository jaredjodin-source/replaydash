#include "RDM.hpp"

#include <fstream>
#include <cstring>

namespace replaydash {

static constexpr char MAGIC[] = "RDM1";

bool RDM::save(
    Macro const& macro,
    std::filesystem::path const& path
) {
    std::ofstream file(
        path,
        std::ios::binary | std::ios::trunc
    );

    if (!file)
        return false;

    file.write(MAGIC, 4);

    uint32_t nameSize =
        static_cast<uint32_t>(
            macro.name.size()
        );

    file.write(
        reinterpret_cast<char const*>(&nameSize),
        sizeof(nameSize)
    );

    if (nameSize != 0) {
        file.write(
            macro.name.data(),
            nameSize
        );
    }

    file.write(
        reinterpret_cast<char const*>(&macro.fps),
        sizeof(macro.fps)
    );

    uint64_t count =
        static_cast<uint64_t>(
            macro.events.size()
        );

    file.write(
        reinterpret_cast<char const*>(&count),
        sizeof(count)
    );

    for (auto const& event : macro.events) {
        uint8_t button =
            static_cast<uint8_t>(event.button);

        uint8_t down =
            event.down ? 1 : 0;

        uint8_t player2 =
            event.player2 ? 1 : 0;

        file.write(
            reinterpret_cast<char const*>(&event.frame),
            sizeof(event.frame)
        );

        file.write(
            reinterpret_cast<char const*>(&button),
            sizeof(button)
        );

        file.write(
            reinterpret_cast<char const*>(&down),
            sizeof(down)
        );

        file.write(
            reinterpret_cast<char const*>(&player2),
            sizeof(player2)
        );
    }

    return file.good();
}

std::optional<Macro> RDM::load(
    std::filesystem::path const& path
) {
    std::ifstream file(
        path,
        std::ios::binary
    );

    if (!file)
        return std::nullopt;

    char magic[4];

    file.read(magic, 4);

    if (
        !file ||
        std::memcmp(magic, MAGIC, 4) != 0
    ) {
        return std::nullopt;
    }

    Macro macro;

    uint32_t nameSize = 0;

    file.read(
        reinterpret_cast<char*>(&nameSize),
        sizeof(nameSize)
    );

    if (
        !file ||
        nameSize > 4096
    ) {
        return std::nullopt;
    }

    macro.name.resize(nameSize);

    if (nameSize != 0) {
        file.read(
            macro.name.data(),
            nameSize
        );
    }

    file.read(
        reinterpret_cast<char*>(&macro.fps),
        sizeof(macro.fps)
    );

    uint64_t count = 0;

    file.read(
        reinterpret_cast<char*>(&count),
        sizeof(count)
    );

    if (
        !file ||
        count > 10000000
    ) {
        return std::nullopt;
    }

    macro.events.reserve(
        static_cast<size_t>(count)
    );

    for (uint64_t i = 0; i < count; ++i) {
        MacroEvent event;

        uint8_t button;
        uint8_t down;
        uint8_t player2;

        file.read(
            reinterpret_cast<char*>(&event.frame),
            sizeof(event.frame)
        );

        file.read(
            reinterpret_cast<char*>(&button),
            sizeof(button)
        );

        file.read(
            reinterpret_cast<char*>(&down),
            sizeof(down)
        );

        file.read(
            reinterpret_cast<char*>(&player2),
            sizeof(player2)
        );

        if (!file)
            return std::nullopt;

        event.button =
            static_cast<MacroButton>(button);

        event.down = down != 0;
        event.player2 = player2 != 0;

        macro.events.push_back(event);
    }

    return macro;
}

}
