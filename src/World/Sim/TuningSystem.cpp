/*
** EPITECH PROJECT, 2026
** racer
** File description:
** TuningSystem implementation
*/

#include "World/Sim/TuningSystem.hpp"

#include "Vehicle/Car.hpp"
#include "World/Sim/ProgressionState.hpp"

namespace racer::world {

namespace {

struct TierMods {
    float maxSpeedMul = 1.0f;
    float accelerationMul = 1.0f;
    float gripMul = 1.0f;
    float brakingMul = 1.0f;
};

// Index 0 is always stock (identity). Costs mirror the tier index they
// unlock (index N = cost to reach tier N).
constexpr std::array<TierMods, kTuningTierCount> kEngineTiers{ {
    TierMods{1.00f, 1.00f, 1.00f, 1.00f},
    TierMods{1.08f, 1.10f, 1.00f, 1.00f},
    TierMods{1.16f, 1.22f, 1.00f, 1.00f},
    TierMods{1.25f, 1.35f, 1.00f, 1.00f},
} };

constexpr std::array<TierMods, kTuningTierCount> kGripTiers{ {
    TierMods{1.00f, 1.00f, 1.00f, 1.00f},
    TierMods{1.00f, 1.00f, 1.12f, 1.00f},
    TierMods{1.00f, 1.00f, 1.24f, 1.00f},
    TierMods{1.00f, 1.00f, 1.38f, 1.00f},
} };

constexpr std::array<TierMods, kTuningTierCount> kBrakesTiers{ {
    TierMods{1.00f, 1.00f, 1.00f, 1.00f},
    TierMods{1.00f, 1.00f, 1.00f, 1.15f},
    TierMods{1.00f, 1.00f, 1.00f, 1.30f},
    TierMods{1.00f, 1.00f, 1.00f, 1.50f},
} };

constexpr std::array<int, kTuningTierCount> kTierCost{ {0, 15, 30, 50} };

const std::array<TierMods, kTuningTierCount> &tiersFor(TuningCategory category)
{
    switch (category) {
    case TuningCategory::GRIP:
        return kGripTiers;
    case TuningCategory::BRAKES:
        return kBrakesTiers;
    case TuningCategory::ENGINE:
    default:
        return kEngineTiers;
    }
}

} // namespace

int TuningState::tier(TuningCategory category) const
{
    size_t i = static_cast<size_t>(category);
    if (i >= tiers_.size()) {
        return 0;
    }
    return tiers_[i];
}

void TuningState::setTier(TuningCategory category, int tier)
{
    size_t i = static_cast<size_t>(category);
    if (i >= tiers_.size()) {
        return;
    }
    tiers_[i] = tier;
}

int TuningSystem::upgradeCost(TuningCategory category, int currentTier)
{
    (void)category;
    if (currentTier < 0 || currentTier >= kTuningMaxTier) {
        return -1;
    }
    return kTierCost[static_cast<size_t>(currentTier) + 1];
}

bool TuningSystem::purchaseUpgrade(TuningState &state, TuningCategory category,
    ProgressionState &progression, RegionId region)
{
    int currentTier = state.tier(category);
    int cost = upgradeCost(category, currentTier);
    if (cost < 0) {
        return false;
    }
    if (!progression.spendReputation(region, cost)) {
        return false;
    }
    state.setTier(category, currentTier + 1);
    return true;
}

void TuningSystem::apply(const TuningState &state, Car &car)
{
    CarTuning t{}; // stock baseline

    const TierMods &engine =
        tiersFor(TuningCategory::ENGINE)[static_cast<size_t>(
            state.tier(TuningCategory::ENGINE))];
    const TierMods &grip =
        tiersFor(TuningCategory::GRIP)[static_cast<size_t>(
            state.tier(TuningCategory::GRIP))];
    const TierMods &brakes =
        tiersFor(TuningCategory::BRAKES)[static_cast<size_t>(
            state.tier(TuningCategory::BRAKES))];

    t.maxSpeed *= engine.maxSpeedMul;
    t.acceleration *= engine.accelerationMul;
    t.gripNormal *= grip.gripMul;
    t.gripDrift *= grip.gripMul;
    t.braking *= brakes.brakingMul;

    car.tuning() = t;
}

} // namespace racer::world
