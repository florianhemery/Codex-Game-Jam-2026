/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurélia open world — replaces legacy OpenWorldHub
*/

#ifndef AURELIA_WORLD_HPP_
#define AURELIA_WORLD_HPP_

#include <vector>

#include "Render/World/WorldRenderer.hpp"
#include "Engine/Render/ShaderLocations.hpp"
#include "Track/Track.hpp"
#include "Vehicle/Car.hpp"
#include "Vehicle/CarInput.hpp"
#include "World/Aurelia/AureliaData.hpp"
#include "World/Sim/AureliaEvents.hpp"
#include "World/Sim/MissionSystem.hpp"
#include "World/Sim/PlayerDriveSystem.hpp"
#include "World/Sim/ProgressionState.hpp"
#include "World/Sim/TrafficSystem.hpp"
#include "World/Sim/TuningSystem.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace racer {
class VfxSystem;
}

namespace racer::world {

class AureliaWorld {
public:
    explicit AureliaWorld(const std::vector<TrackDef> &presets,
        RegionId spawnRegion = RegionId::MARINA,
        const ProgressionState *savedProgression = nullptr);

    void update(float dt, const CarInput &input, float steerSmoothed,
        VfxSystem *vfx = nullptr);

    const Car &playerCar() const { return playerCar_; }
    float steerSmoothed() const { return steerSmoothed_; }
    float wheelSpin() const { return wheelSpin_; }
    float timeOfDay() const { return timeOfDay_; }

    int activeRaceIndex() const { return activeRace_; }
    const PoiInstance *activeRacePoi() const;
    const PoiInstance *activeMissionPoi() const;
    const PoiInstance *activeGaragePoi() const;
    int activeCollectibleIndex() const { return activeCollectible_; }

    const TuningState &tuning() const { return tuningState_; }
    int tuningTier(TuningCategory category) const
    {
        return tuningState_.tier(category);
    }
    int tuningUpgradeCost(TuningCategory category) const
    {
        return TuningSystem::upgradeCost(category, tuningState_.tier(category));
    }
    bool purchaseUpgrade(TuningCategory category);

    BiomeId currentBiome() const;
    racer::engine::Ambiance ambianceForTime() const;

    ChunkStreamer &streamer() { return streamer_; }
    const ChunkStreamer &streamer() const { return streamer_; }
    WorldRenderer &renderer() { return renderer_; }
    const WorldRenderer &renderer() const { return renderer_; }
    MissionSystem &missions() { return missions_; }
    const MissionSystem &missions() const { return missions_; }
    ProgressionState &progression() { return progression_; }
    const ProgressionState &progression() const { return progression_; }
    TrafficSystem &traffic() { return traffic_; }

    void drawOpaque();
    void drawLit(float timeSec);
    void drawTriggers();
    void drawTraffic() const;
    void applyShader(Shader shader);

    float fogDensity() const;
    float pipelineFogDensity() const;
    bool showWorldMap() const { return showWorldMap_; }
    void toggleWorldMap() { showWorldMap_ = !showWorldMap_; }

    const std::vector<PoiInstance> &pois() const;

    // Scripted "living world" moments (convoys, weather events). A future
    // HUD pass can surface this label; the underlying fog/rain/traffic
    // effect is already real and applied every frame.
    bool hasActiveWorldEvent() const { return events_.hasActiveEvent(); }
    const char *activeEventLabel() const { return events_.activeEventLabel(); }
    float activeEventTimeLeft() const { return events_.activeEventTimeLeft(); }
    bool activeEventIsConvoy() const
    {
        return events_.activeEventWantsConvoyBurst();
    }

private:
    void updateActivePois();
    void collectNearbyLore();
    void applyRegionalVfx(VfxSystem *vfx) const;
    void respawnAtGarage(RegionId region);

    Car playerCar_;
    ChunkStreamer streamer_;
    WorldRenderer renderer_;
    ProgressionState progression_;
    MissionSystem missions_;
    TrafficSystem traffic_;
    PlayerDriveSystem drive_;
    AureliaEvents events_;

    float steerSmoothed_ = 0.0f;
    float wheelSpin_ = 0.0f;
    float timeOfDay_ = 0.35f;
    int activeRace_ = -1;
    int activeMissionPoi_ = -1;
    int activeCollectible_ = -1;
    int activeGaragePoi_ = -1;
    bool showWorldMap_ = false;
    TuningState tuningState_{};
};

} // namespace racer::world

#endif /* !AURELIA_WORLD_HPP_ */
