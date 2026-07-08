/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurélia decor — arbres, portails circuit
*/

#include "Render/World/WorldDecorDraw.hpp"

namespace racer::world {

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
