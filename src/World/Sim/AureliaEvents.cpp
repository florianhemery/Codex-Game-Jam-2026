/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Scripted "living world" events (convoys, weather moments) for Aurélia
*/

#include "World/Sim/AureliaEvents.hpp"

#include "Render/Vfx/VfxSystem.hpp"

namespace racer::world {

namespace {

// Data-driven event table: one row per event, no per-event special-case
// code. Adding a 6th event is a new row here plus (optionally) reading its
// flags where the effect is applied.
constexpr AureliaEventDef kEventDefs[] = {
    {
        AureliaEventId::CONVOI_DU_SOIR,
        "Convoi du soir",
        BiomeId::PORT,
        0.78f, 0.90f, // dusk window
        40.0f,        // duration (s)
        120.0f,       // cooldown (s)
        0.0f,         // fogBoost
        false,        // forceRain
        true,         // convoyBurst
    },
    {
        AureliaEventId::TEMPETE_DE_CENDRES,
        "Tempête de cendres",
        BiomeId::VOLCANO,
        0.45f, 0.55f, // matches the existing ORAGE ambiance window
        45.0f,
        150.0f,
        0.20f,
        true,
        false,
    },
    {
        AureliaEventId::BRUME_DENSE,
        "Brume dense",
        BiomeId::FOREST,
        0.0f, 0.15f, // deep night
        60.0f,
        180.0f,
        0.28f,
        false,
        false,
    },
    {
        AureliaEventId::MAREE_HAUTE,
        "Marée haute",
        BiomeId::COAST,
        0.55f, 0.65f, // late afternoon
        30.0f,
        120.0f,
        0.08f,
        false,
        false,
    },
    {
        AureliaEventId::CONVOI_TITAN,
        "Convoi titan",
        BiomeId::VOLCANO,
        0.30f, 0.35f, // rare mid-morning window
        50.0f,
        240.0f,
        0.0f,
        false,
        true,
    },
};

constexpr int kEventCount = static_cast<int>(
    sizeof(kEventDefs) / sizeof(kEventDefs[0]));

} // namespace

AureliaEvents::AureliaEvents()
{
    static_assert(kEventCount == static_cast<int>(AureliaEventId::COUNT),
        "kEventDefs must have exactly one row per AureliaEventId");
}

bool AureliaEvents::inWindow(float timeOfDay, float start, float end)
{
    if (start <= end) {
        return timeOfDay >= start && timeOfDay < end;
    }
    // Wrapping window (e.g. crosses midnight): active outside [end, start).
    return timeOfDay >= start || timeOfDay < end;
}

const AureliaEventDef *AureliaEvents::findDef(AureliaEventId id) const
{
    for (const AureliaEventDef &def : kEventDefs) {
        if (def.id == id) {
            return &def;
        }
    }
    return nullptr;
}

void AureliaEvents::tryTrigger(float timeOfDay, BiomeId biome)
{
    for (const AureliaEventDef &def : kEventDefs) {
        int idx = static_cast<int>(def.id);
        if (runtime_[idx].cooldownLeft > 0.0f) {
            continue;
        }
        if (def.biome != biome) {
            continue;
        }
        if (!inWindow(timeOfDay, def.timeStart, def.timeEnd)) {
            continue;
        }
        active_ = def.id;
        activeTimeLeft_ = def.duration;
        runtime_[idx].cooldownLeft = def.duration + def.cooldown;
        return;
    }
}

void AureliaEvents::update(float dt, float timeOfDay, BiomeId biome,
    VfxSystem *vfx)
{
    for (AureliaEventRuntime &rt : runtime_) {
        if (rt.cooldownLeft > 0.0f) {
            rt.cooldownLeft -= dt;
        }
    }

    if (active_ != AureliaEventId::NONE) {
        activeTimeLeft_ -= dt;
        if (activeTimeLeft_ <= 0.0f) {
            active_ = AureliaEventId::NONE;
            activeTimeLeft_ = 0.0f;
        }
    }

    if (active_ == AureliaEventId::NONE) {
        tryTrigger(timeOfDay, biome);
    }

    if (active_ != AureliaEventId::NONE && vfx) {
        const AureliaEventDef *def = findDef(active_);
        if (def) {
            if (def->fogBoost > 0.0f) {
                vfx->setFogDensity(vfx->fogDensity() + def->fogBoost);
            }
            if (def->forceRain) {
                vfx->setRain(true);
            }
        }
    }
}

const char *AureliaEvents::activeEventLabel() const
{
    const AureliaEventDef *def = findDef(active_);
    return def ? def->label : "";
}

bool AureliaEvents::activeEventWantsConvoyBurst() const
{
    const AureliaEventDef *def = findDef(active_);
    return def && def->convoyBurst;
}

} // namespace racer::world
