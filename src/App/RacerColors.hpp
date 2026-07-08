/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Racer palette and track ambiance mapping
*/

#ifndef RACER_COLORS_HPP_
#define RACER_COLORS_HPP_

#include <cstddef>

#include "raylib.h"

#include "Engine/Render/RenderPipeline.hpp"
#include "Track/TrackDef.hpp"

namespace racer {
namespace app {

Color colorForRacerIndex(size_t index, bool isPlayer);
engine::Ambiance ambianceForTrack(int trackIndex, const TrackDef &def);

const char *ambianceLabelForTrack(int trackIndex, const TrackDef &def);

} // namespace app
} // namespace racer

#endif
