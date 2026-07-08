/*
** EPITECH PROJECT, 2026
** racer
** File description:
** AureliaWorld main simulation loop
*/

#include "World/Aurelia/AureliaWorld.hpp"

#include <cmath>

#include "raylib.h"

#include "Engine/Render/ShaderLocations.hpp"
#include "Render/VfxSystem.hpp"

namespace racer::world {

AureliaWorld::AureliaWorld(const std::vector<TrackDef> &presets,
    RegionId spawnRegion, const ProgressionState *savedProgression)
    : missions_(progression_)
    , traffic_(AureliaData::roadGraph())
{
    AureliaData::initCollectibles();
    AureliaData::attachRaceLabels(presets);
    if (savedProgression) {
        progression_ = *savedProgression;
    }

    float spawnX = 16.0f;
    float spawnZ = -8.0f;
    for (const PoiInstance &poi : AureliaData::worldPois()) {
        if (poi.type == PoiType::GARAGE && poi.region == spawnRegion) {
            spawnX = poi.worldX;
            spawnZ = poi.worldZ;
            break;
        }
    }
    drive_.reset(playerCar_, spawnX, spawnZ, 0.0f);
    streamer_.updateCenter(playerCar_.position().x, playerCar_.position().z);
    streamer_.ensureLoaded();
    renderer_.sync(streamer_);
}

void AureliaWorld::update(float dt, const CarInput &input, float steerSmoothed,
    VfxSystem *vfx)
{
    steerSmoothed_ = steerSmoothed;
    timeOfDay_ += dt * 0.002f;
    if (timeOfDay_ > 1.0f) {
        timeOfDay_ -= 1.0f;
    }

    drive_.update(playerCar_, input, steerSmoothed, dt, streamer_, wheelSpin_);

    float px = playerCar_.position().x;
    float pz = playerCar_.position().z;
    streamer_.updateCenter(px, pz);
    streamer_.ensureLoaded();
    renderer_.sync(streamer_);

    traffic_.update(dt, streamer_, px, pz);
    missions_.update(dt, px, pz, playerCar_.speed());
    updateActivePois();
    collectNearbyLore();
    applyRegionalVfx(vfx);
}

void AureliaWorld::updateActivePois()
{
    activeRace_ = -1;
    activeMissionPoi_ = -1;
    activeCollectible_ = -1;

    float px = playerCar_.position().x;
    float pz = playerCar_.position().z;

    for (size_t i = 0; i < AureliaData::worldPois().size(); ++i) {
        const PoiInstance &poi = AureliaData::worldPois()[i];
        float dx = px - poi.worldX;
        float dz = pz - poi.worldZ;
        float dist2 = dx * dx + dz * dz;
        if (dist2 > poi.radius * poi.radius) {
            continue;
        }
        switch (poi.type) {
        case PoiType::RACE_ENTRY:
            if (poi.trackIndex == 4
                && !progression_.cendresCircuitUnlocked()) {
                break;
            }
            activeRace_ = poi.trackIndex;
            break;
        case PoiType::MISSION_GIVER:
            activeMissionPoi_ = static_cast<int>(i);
            break;
        case PoiType::COLLECTIBLE:
            activeCollectible_ = static_cast<int>(i);
            break;
        default:
            break;
        }
    }
}

void AureliaWorld::collectNearbyLore()
{
    if (activeCollectible_ < 0) {
        return;
    }
    const PoiInstance &poi =
        AureliaData::worldPois()[static_cast<size_t>(activeCollectible_)];
    if (poi.loreIndex >= 0) {
        progression_.collectLore(poi.loreIndex);
    }
}

void AureliaWorld::applyRegionalVfx(VfxSystem *vfx) const
{
    if (!vfx) {
        return;
    }
    BiomeId biome = currentBiome();
    vfx->setRain(biome == BiomeId::PORT);
    vfx->setFogDensity(fogDensity());
}

const PoiInstance *AureliaWorld::activeRacePoi() const
{
    if (activeRace_ < 0) {
        return nullptr;
    }
    for (const PoiInstance &poi : AureliaData::worldPois()) {
        if (poi.type == PoiType::RACE_ENTRY && poi.trackIndex == activeRace_) {
            return &poi;
        }
    }
    return nullptr;
}

const PoiInstance *AureliaWorld::activeMissionPoi() const
{
    if (activeMissionPoi_ < 0) {
        return nullptr;
    }
    return &AureliaData::worldPois()[static_cast<size_t>(activeMissionPoi_)];
}

BiomeId AureliaWorld::currentBiome() const
{
    return streamer_.biomeAt(playerCar_.position().x, playerCar_.position().z);
}

racer::engine::Ambiance AureliaWorld::ambianceForTime() const
{
    if (timeOfDay_ < 0.25f || timeOfDay_ > 0.85f) {
        return racer::engine::Ambiance::CREPUSCULE;
    }
    if (timeOfDay_ > 0.45f && timeOfDay_ < 0.55f
        && currentBiome() == BiomeId::VOLCANO) {
        return racer::engine::Ambiance::ORAGE;
    }
    if (timeOfDay_ < 0.35f) {
        return racer::engine::Ambiance::AUBE_DOREE;
    }
    return racer::engine::Ambiance::MIDI;
}

float AureliaWorld::fogDensity() const
{
    BiomeId biome = currentBiome();
    if (biome == BiomeId::FOREST) {
        return 0.22f;
    }
    if (biome == BiomeId::VOLCANO) {
        return 0.14f;
    }
    if (biome == BiomeId::PORT) {
        return 0.10f;
    }
    return 0.06f;
}

float AureliaWorld::pipelineFogDensity() const
{
    BiomeId biome = currentBiome();
    if (biome == BiomeId::FOREST) {
        return 0.0024f;
    }
    if (biome == BiomeId::VOLCANO) {
        return 0.0045f;
    }
    if (biome == BiomeId::PORT) {
        return 0.0048f;
    }
    return 0.0020f;
}

void AureliaWorld::drawOpaque()
{
    renderer_.drawOpaque(currentBiome());
}

void AureliaWorld::drawLit(float timeSec)
{
    renderer_.drawLit(timeSec, playerCar_.position(), streamer_);
}

void AureliaWorld::applyShader(Shader shader)
{
    renderer_.applyShader(shader);
}

void AureliaWorld::drawTriggers()
{
    renderer_.drawTriggers(static_cast<float>(GetTime()),
        playerCar_.position(), streamer_);
}

void AureliaWorld::drawTraffic() const
{
    renderer_.drawTraffic(traffic_.vehicles(), AureliaData::roadGraph(),
        playerCar_.position(), streamer_);
}

const std::vector<PoiInstance> &AureliaWorld::pois() const
{
    return AureliaData::worldPois();
}

void AureliaWorld::respawnAtGarage(RegionId region)
{
    for (const PoiInstance &poi : AureliaData::worldPois()) {
        if (poi.type == PoiType::GARAGE && poi.region == region) {
            drive_.reset(playerCar_, poi.worldX, poi.worldZ, 0.0f);
            return;
        }
    }
}

} // namespace racer::world
