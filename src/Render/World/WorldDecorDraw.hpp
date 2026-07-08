/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurélia world prop and landmark drawing (style piste)
*/

#ifndef WORLD_DECOR_DRAW_HPP_
#define WORLD_DECOR_DRAW_HPP_

#include "raylib.h"

#include "Render/World/WorldPropBuilder.hpp"
#include "World/Aurelia/AureliaTypes.hpp"
#include "World/Chunk/ChunkData.hpp"

namespace racer::world {

class WorldDecorDraw {
public:
    static void drawRaceGate(WorldPropBuilder &builder, const PoiInstance &poi,
        float groundY, float timeSec);
    static void drawGarage(WorldPropBuilder &builder, const PoiInstance &poi,
        float groundY);
    static void drawMissionMarker(WorldPropBuilder &builder,
        const PoiInstance &poi, float groundY, float timeSec);
};

} // namespace racer::world

#endif /* !WORLD_DECOR_DRAW_HPP_ */
