#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "Macro.hpp"
#include "RDM.hpp"
#include "GDR.hpp"
#include "GDR2.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>

using namespace geode::prelude;

namespace replaydash {

static Macro g_macro;

static bool g_recording = false;
static bool g_playing = false;

static uint64_t g_frame = 0;
static size_t g_playIndex = 0;

static std::filesystem::path getSaveDirectory() {
    auto dir =
        Mod::get()->getSaveDir();

    std::filesystem::create_directories(dir);

    return dir;
}

static MacroButton toMacroButton(int button) {
    switch (button) {
        case 2:
            return MacroButton::Left;

        case 3:
            return MacroButton::Right;

        default:
            return MacroButton::Jump;
    }
}

static int toGameButton(
    MacroButton button
) {
    switch (button) {
        case MacroButton::Left:
            return 2;

        case MacroButton::Right:
            return 3;

        default:
            return 1;
    }
}

static void startRecording() {
    g_macro.clear();

    g_macro.name =
        "ReplayDash Recording";

    g_macro.fps = 240.0;

    g_frame = 0;
    g_playIndex = 0;

    g_recording = true;
    g_playing = false;

    log::info(
        "ReplayDash: recording started"
    );
}

static void stopRecording() {
    g_recording = false;

    auto path =
        getSaveDirectory() /
        "last_recording.rdm";

    if (
        RDM::save(
            g_macro,
            path
        )
    ) {
        log::info(
            "ReplayDash: saved {}",
            path.string()
        );
    }
    else {
        log::error(
            "ReplayDash: could not save macro"
        );
    }
}

static void startPlayback(
    Macro macro
) {
    if (macro.events.empty()) {
        log::error(
            "ReplayDash: macro is empty"
        );

        return;
    }

    std::sort(
        macro.events.begin(),
        macro.events.end(),
        [](auto const& a, auto const& b) {
            return a.frame < b.frame;
        }
    );

    g_macro =
        std::move(macro);

    g_frame = 0;
    g_playIndex = 0;

    g_recording = false;
    g_playing = true;

    log::info(
        "ReplayDash: playback started"
    );
}

static void stopPlayback() {
    g_playing = false;
    g_playIndex = 0;

    log::info(
        "ReplayDash: playback stopped"
    );
}

static void playLastRecording() {
    auto path =
        getSaveDirectory() /
        "last_recording.rdm";

    auto macro =
        RDM::load(path);

    if (!macro) {
        log::error(
            "ReplayDash: no valid RDM recording found"
        );

        return;
    }

    startPlayback(
        std::move(*macro)
    );
}

class $modify(
    ReplayDashGameLayer,
    GJBaseGameLayer
) {

    void handleButton(
        bool down,
        int button,
        bool isPlayer1
    ) {

        /*
         * Enregistrement.
         */

        if (g_recording) {
            MacroEvent event;

            event.frame =
                g_frame;

            event.button =
                toMacroButton(button);

            event.down =
                down;

            event.player2 =
                !isPlayer1;

            g_macro.events.push_back(
                event
            );
        }

        /*
         * Toujours laisser GD traiter
         * l'input normalement.
         */

        GJBaseGameLayer::handleButton(
            down,
            button,
            isPlayer1
        );
    }

    void update(float dt) {

        GJBaseGameLayer::update(dt);

        ++g_frame;

        if (!g_playing)
            return;

        while (
            g_playIndex <
            g_macro.events.size()
        ) {
            auto const& event =
                g_macro.events[g_playIndex];

            if (
                event.frame >
                g_frame
            ) {
                break;
            }

            /*
             * On injecte directement dans
             * le moteur GD sans repasser
             * par notre hook d'enregistrement.
             */

            GJBaseGameLayer::handleButton(
                event.down,
                toGameButton(event.button),
                !event.player2
            );

            ++g_playIndex;
        }

        if (
            g_playIndex >=
            g_macro.events.size()
        ) {
            stopPlayback();
        }
    }
};

$execute {
    log::info(
        "ReplayDash v1.0.0 loaded"
    );

    /*
     * Pour l'instant les fonctions sont
     * disponibles dans le moteur.
     *
     * Le prochain composant est le menu UI.
     */

    auto save =
        getSaveDirectory() /
        "last_recording.rdm";

    if (
        std::filesystem::exists(save)
    ) {
        log::info(
            "ReplayDash: previous recording found"
        );
    }
}

}
