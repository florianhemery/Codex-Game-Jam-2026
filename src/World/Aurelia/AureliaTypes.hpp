/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurélia POI, mission and region types
*/

#ifndef AURELIA_TYPES_HPP_
#define AURELIA_TYPES_HPP_

#include <cstdint>

#include "raylib.h"

#include "World/Chunk/ChunkTypes.hpp"

namespace racer::world {

enum class PoiType : std::uint8_t {
    RACE_ENTRY = 0,
    GARAGE,
    MISSION_GIVER,
    VIEWPOINT,
    COLLECTIBLE,
    GARAGE_COSMETIC
};

enum class RegionId : std::uint8_t {
    MARINA = 0,
    FOREST,
    PORT,
    VOLCANO,
    COUNT
};

enum class MissionKind : std::uint8_t {
    CONVOY = 0,
    GHOST_CHASE,
    DELIVERY,
    FOG_ESCAPE,
    CALDERA_CLIMB,
    TIME_TRIAL
};

enum class MissionState : std::uint8_t {
    LOCKED = 0,
    AVAILABLE,
    ACTIVE,
    COMPLETED
};

struct PoiInstance {
    const char *id = "";
    PoiType type = PoiType::RACE_ENTRY;
    const char *label = "";
    float worldX = 0.0f;
    float worldZ = 0.0f;
    float radius = 12.0f;
    int trackIndex = -1;
    int missionIndex = -1;
    int loreIndex = -1;
    RegionId region = RegionId::MARINA;
    Color color = WHITE;
};

struct MissionDef {
    const char *id = "";
    const char *title = "";
    const char *description = "";
    MissionKind kind = MissionKind::CONVOY;
    RegionId region = RegionId::MARINA;
    float targetTime = 120.0f;
    int rewardRep = 10;
    BiomeId biome = BiomeId::COAST;
};

RegionId regionForBiome(BiomeId biome);
BiomeId biomeForChunk(int chunkX, int chunkZ);

} // namespace racer::world

#endif /* !AURELIA_TYPES_HPP_ */
