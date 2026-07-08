/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurélia decor — arbres, portails circuit
*/

#include "Render/World/WorldDecorDraw.hpp"

namespace racer::world {

void WorldDecorDraw::drawProp(WorldPropBuilder &builder, Vector3 worldPos,
    std::uint8_t type, float yaw, float scale, BiomeId biome)
{
    builder.placeScatterProp(type, worldPos, yaw, scale, biome);
}

void WorldDecorDraw::drawMarinaLandmarks(WorldPropBuilder &builder,
    float groundY)
{
    builder.placeMarinaLandmarks(groundY);
}

void WorldDecorDraw::drawRaceGate(WorldPropBuilder &builder,
    const PoiInstance &poi, float groundY, float timeSec)
{
    builder.placeRaceGate(poi, groundY, timeSec);
}

void WorldDecorDraw::drawGarage(WorldPropBuilder &builder,
    const PoiInstance &poi, float groundY)
{
    builder.placeGarage(poi, groundY);
}

void WorldDecorDraw::drawMissionMarker(WorldPropBuilder &builder,
    const PoiInstance &poi, float groundY, float timeSec)
{
    builder.placeMissionMarker(poi, groundY, timeSec);
}

} // namespace racer::world
