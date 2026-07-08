/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Scripted "living world" events (convoys, weather moments) for Aurélia
*/

#ifndef AURELIA_EVENTS_HPP_
#define AURELIA_EVENTS_HPP_

#include "World/Chunk/ChunkTypes.hpp"

namespace racer {
class VfxSystem;
}

namespace racer::world {

enum class AureliaEventId : int {
    NONE = -1,
    CONVOI_DU_SOIR = 0,
    TEMPETE_DE_CENDRES,
    BRUME_DENSE,
    MAREE_HAUTE,
    CONVOI_TITAN,
    COUNT
};

// A single scripted moment: a time-of-day window + biome gate that, once
// triggered, runs for a fixed duration with a real, observable effect
// (fog/rain boost above the ambient baseline, or a traffic convoy burst).
// Table-driven so a 6th/7th event is just a new row, not new logic.
struct AureliaEventDef {
    AureliaEventId id = AureliaEventId::NONE;
    const char *label = "";
    BiomeId biome = BiomeId::COAST;
    float timeStart = 0.0f;   // inclusive, in [0,1)
    float timeEnd = 1.0f;     // exclusive, in [0,1); may wrap (start > end)
    float duration = 30.0f;   // seconds the event stays active once triggered
    float cooldown = 90.0f;   // seconds before it can trigger again
    float fogBoost = 0.0f;    // added on top of ambient fogDensity()
    bool forceRain = false;   // forces rain on for the duration
    bool convoyBurst = false; // signals a traffic convoy grouping
};

// Runtime state for one event definition: has it triggered, how long is
// left, and how long until it is eligible again.
struct AureliaEventRuntime {
    float activeTimeLeft = 0.0f;
    float cooldownLeft = 0.0f;
};

// Lightweight scheduler: evaluates the event table every frame against the
// current time-of-day/biome, activates at most one event at a time, applies
// its effect to VfxSystem, and exposes a label for a future HUD pass.
class AureliaEvents {
public:
    AureliaEvents();

    void update(float dt, float timeOfDay, BiomeId biome, VfxSystem *vfx);

    bool hasActiveEvent() const { return active_ != AureliaEventId::NONE; }
    AureliaEventId activeEventId() const { return active_; }
    const char *activeEventLabel() const;
    bool activeEventWantsConvoyBurst() const;
    float activeEventTimeLeft() const { return activeTimeLeft_; }

private:
    static bool inWindow(float timeOfDay, float start, float end);
    const AureliaEventDef *findDef(AureliaEventId id) const;
    void tryTrigger(float timeOfDay, BiomeId biome);

    AureliaEventRuntime runtime_[static_cast<int>(AureliaEventId::COUNT)];
    AureliaEventId active_ = AureliaEventId::NONE;
    float activeTimeLeft_ = 0.0f;
};

} // namespace racer::world

#endif /* !AURELIA_EVENTS_HPP_ */
