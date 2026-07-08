/*
** EPITECH PROJECT, 2026
** racer
** File description:
** AureliaTypes helpers
*/

#include "World/Aurelia/AureliaTypes.hpp"

namespace racer::world {

RegionId regionForBiome(BiomeId biome)
{
    switch (biome) {
    case BiomeId::FOREST:
        return RegionId::FOREST;
    case BiomeId::PORT:
        return RegionId::PORT;
    case BiomeId::VOLCANO:
        return RegionId::VOLCANO;
    default:
        return RegionId::MARINA;
    }
}

BiomeId biomeForChunk(int chunkX, int chunkZ)
{
    if (chunkZ >= 1) {
        return BiomeId::VOLCANO;
    }
    if (chunkX >= 1 && chunkZ <= 0) {
        return BiomeId::PORT;
    }
    if (chunkX <= -1 || chunkZ <= -1) {
        return BiomeId::FOREST;
    }
    return BiomeId::COAST;
}

} // namespace racer::world
