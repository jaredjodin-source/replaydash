#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace replaydash {

enum class MacroButton : uint8_t {
    Jump = 1,
    Left = 2,
    Right = 3
};

struct MacroEvent {
    uint64_t frame = 0;
    MacroButton button = MacroButton::Jump;
    bool down = false;
    bool player2 = false;
};

struct Macro {
    std::string name = "ReplayDash";

    double fps = 240.0;

    std::vector<MacroEvent> events;

    void clear();

    bool empty() const;
};

}
