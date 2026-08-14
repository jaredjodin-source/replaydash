#include "GDR.hpp"

#include <Geode/Geode.hpp>

#include <fstream>
#include <sstream>

using namespace geode::prelude;

namespace replaydash {

static MacroButton convertButton(int button) {
    switch (button) {
        case 2:
            return MacroButton::Left;

        case 3:
            return MacroButton::Right;

        default:
            return MacroButton::Jump;
    }
}

std::optional<Macro> GDR::load(
    std::filesystem::path const& path
) {
    std::ifstream file(
        path,
        std::ios::binary
    );

    if (!file)
        return std::nullopt;

    std::stringstream buffer;
    buffer << file.rdbuf();

    auto text = buffer.str();

    if (text.empty())
        return std::nullopt;

    /*
     * GDR JSON.
     *
     * MessagePack n'est volontairement pas
     * traité ici.
     */

    if (text.front() != '{')
        return std::nullopt;

    try {
        auto json =
            matjson::parse(text);

        Macro macro;

        if (json.contains("framerate")) {
            macro.fps =
                json["framerate"]
                    .asDouble()
                    .unwrapOr(240.0);
        }

        if (!json.contains("inputs"))
            return std::nullopt;

        auto inputs =
            json["inputs"].asArray();

        if (!inputs)
            return std::nullopt;

        for (auto const& input : inputs.unwrap()) {
            MacroEvent event;

            event.frame =
                static_cast<uint64_t>(
                    input["frame"]
                        .asInt()
                        .unwrapOr(0)
                );

            event.button =
                convertButton(
                    input["btn"]
                        .asInt()
                        .unwrapOr(1)
                );

            event.down =
                input["down"]
                    .asBool()
                    .unwrapOr(false);

            event.player2 =
                input["2p"]
                    .asBool()
                    .unwrapOr(false);

            macro.events.push_back(event);
        }

        if (macro.events.empty())
            return std::nullopt;

        macro.name = "Imported GDR";

        return macro;
    }
    catch (...) {
        return std::nullopt;
    }
}

}
