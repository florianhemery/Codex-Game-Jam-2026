/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track sky gradient background
*/

#include "Render/Track/TrackSkyDraw.hpp"

#include "Render/Track/TrackMeshBuilder.hpp"

namespace racer {

void TrackSkyDraw::drawGradient(int screenWidth, int screenHeight)
{
    Color horizon{135, 196, 235, 255};
    Color zenith{42, 92, 168, 255};

    ClearBackground(horizon);
    int bands = 32;
    int bandH = (screenHeight + bands - 1) / bands;

    for (int i = 0; i < bands; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(bands - 1);
        Color c = TrackMeshBuilder::lerpColor(horizon, zenith, t * t);

        DrawRectangle(0, i * bandH, screenWidth, bandH + 1, c);
    }
}

} // namespace racer
