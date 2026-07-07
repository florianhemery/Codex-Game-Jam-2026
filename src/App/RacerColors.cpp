/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Racer palette and track ambiance mapping
*/

#include "App/RacerColors.hpp"

namespace racer {
namespace app {

Color colorForRacerIndex(size_t index, bool isPlayer)
{
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

} // namespace app
} // namespace racer
