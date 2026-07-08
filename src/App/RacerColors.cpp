/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Racer palette and track ambiance mapping
*/

#include "App/RacerColors.hpp"

#include "Render/Hud/HudGfx.hpp"

namespace racer {
namespace app {

Color colorForRacerIndex(size_t index, bool isPlayer)
{
    if (HudGfx::colorblindMode()) {
        // Okabe-Ito colorblind-safe palette (see HudGfx::racerColorFor for
        // the same set used when no per-race extras.racerColors is set).
        if (isPlayer)
            return Color{213, 94, 0, 255};
        constexpr Color cbPalette[] = {
            Color{0, 114, 178, 255}, Color{0, 158, 115, 255},
            Color{204, 121, 167, 255}
        };
        return cbPalette[index % (sizeof(cbPalette) / sizeof(cbPalette[0]))];
    }
    if (isPlayer)
        return RED;
    constexpr Color palette[] = {BLUE, DARKGREEN, ORANGE, PURPLE};

    return palette[index % (sizeof(palette) / sizeof(palette[0]))];
}

engine::Ambiance ambianceForTrack(int trackIndex, const TrackDef &def)
{
    if (def.surfaceStyle == SurfaceStyle::ABIMEE)
        return engine::Ambiance::ORAGE;
    switch (trackIndex % 3) {
    case 0:
        return engine::Ambiance::MIDI;
    case 1:
        return engine::Ambiance::AUBE_DOREE;
    default:
        return engine::Ambiance::CREPUSCULE;
    }
}

const char *ambianceLabelForTrack(int trackIndex, const TrackDef &def)
{
    if (def.surfaceStyle == SurfaceStyle::ABIMEE)
        return "Pluie / route abimee";
    switch (trackIndex % 3) {
    case 0:
        return "Midi ensoleille";
    case 1:
        return "Aube doree";
    default:
        return "Crepuscule";
    }
}

} // namespace app
} // namespace racer
