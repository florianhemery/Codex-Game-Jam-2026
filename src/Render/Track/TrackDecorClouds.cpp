/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track cloud puff and grandstand spectator setup
*/

#include <cmath>

#include "Render/Track/TrackDecorBuilder.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Render/Track/TrackRenderer.hpp"
#include "rlgl.h"

namespace racer {

void TrackDecorBuilder::buildCloudPuffs(
    TrackCloudInstance &cloud, int cloudIndex)
{
    uint32_t h = TrackMeshBuilder::hashIndex(
        static_cast<size_t>(cloudIndex) + 9000);
    int puffCount = 3 + static_cast<int>(h % 3);

    for (int p = 0; p < puffCount; ++p) {
        uint32_t ph = TrackMeshBuilder::hashIndex(
            static_cast<size_t>(cloudIndex * 10 + p));
        cloud.puffOffsets.push_back(Vector3{
            static_cast<float>(ph % 100) * 0.08f - 4.0f,
            static_cast<float>((ph >> 4) % 50) * 0.02f,
            static_cast<float>((ph >> 8) % 100) * 0.08f - 4.0f,
        });
        cloud.puffScales.push_back(
            1.8f + static_cast<float>(ph % 40) * 0.04f);
    }
}

void TrackDecorBuilder::buildCloudRing(TrackRenderer &renderer)
{
    for (int c = 0; c < 20; ++c) {
        uint32_t h = TrackMeshBuilder::hashIndex(
            static_cast<size_t>(c) + 9000);
        float angle = static_cast<float>(c) / 20.0f * 2.0f * PI;
        float dist = renderer.groundSpan_ * 0.55f
            + static_cast<float>(h % 120);
        TrackCloudInstance cloud;

        cloud.basePosition = Vector3{
            std::cos(angle) * dist, 42.0f + static_cast<float>(h % 20),
            std::sin(angle) * dist,
        };
        cloud.driftSpeed = 0.4f + static_cast<float>(h % 50) * 0.01f;
        cloud.scale = 2.5f + static_cast<float>(h % 30) * 0.05f;
        buildCloudPuffs(cloud, c);
        renderer.clouds_.push_back(cloud);
    }
}

Color TrackDecorBuilder::makeSpectatorShirtColor(uint32_t sh)
{
    return Color{
        static_cast<unsigned char>(80 + sh % 175),
        static_cast<unsigned char>(60 + (sh >> 4) % 175),
        static_cast<unsigned char>(50 + (sh >> 8) % 175),
        255,
    };
}

Vector3 TrackDecorBuilder::makeSpectatorPosition(
    const TrackGrandstandInstance &gs,
    float rowH, float alongT, float outwardD)
{
    return Vector3{
        gs.origin.x + gs.along.x * alongT + gs.outward.x * outwardD,
        rowH,
        gs.origin.z + gs.along.z * alongT + gs.outward.z * outwardD,
    };
}

void TrackDecorBuilder::addGrandstandSpectator(
    TrackGrandstandInstance &gs, size_t mid,
    int row, int col)
{
    constexpr int kCols = 14;
    uint32_t sh = TrackMeshBuilder::hashIndex(
        static_cast<size_t>(row * kCols + col) + mid * 17);
    float alongT = (static_cast<float>(col) / (kCols - 1) - 0.5f) * gs.length;
    float outwardD = static_cast<float>(row) * 1.8f + 1.5f;
    float rowH = static_cast<float>(row) * 1.1f + 0.5f;
    TrackSpectatorInstance spec;

    spec.position = makeSpectatorPosition(gs, rowH, alongT, outwardD);
    spec.shirtColor = makeSpectatorShirtColor(sh);
    spec.jumpPhase = static_cast<float>(sh % 628) * 0.01f;
    spec.jumpSpeed = 2.5f + static_cast<float>(sh % 30) * 0.05f;
    gs.spectators.push_back(spec);
}

void TrackDecorBuilder::fillGrandstandSpectators(
    TrackGrandstandInstance &gs, size_t mid)
{
    constexpr int kRows = 4;
    constexpr int kCols = 14;

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col)
            addGrandstandSpectator(gs, mid, row, col);
    }
}

} // namespace racer
