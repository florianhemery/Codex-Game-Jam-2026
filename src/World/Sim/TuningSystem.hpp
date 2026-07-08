/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Garage upgrade categories, tiers and Car stat application
*/

#ifndef TUNING_SYSTEM_HPP_
#define TUNING_SYSTEM_HPP_

#include <array>

#include "World/Aurelia/AureliaTypes.hpp"

namespace racer {
class Car;
}

namespace racer::world {

class ProgressionState;

enum class TuningCategory : int {
    ENGINE = 0,
    GRIP,
    BRAKES,
    COUNT
};

constexpr int kTuningTierCount = 4; // stock + 3 upgrade tiers
constexpr int kTuningMaxTier = kTuningTierCount - 1;

// Discrete purchasable state of a car's upgrades. All tiers start at 0
// (stock). Persisted per-run inside AureliaWorld / save data.
class TuningState {
public:
    int tier(TuningCategory category) const;
    void setTier(TuningCategory category, int tier);

private:
    std::array<int, static_cast<size_t>(TuningCategory::COUNT)> tiers_{};
};

class TuningSystem {
public:
    // Reputation cost required to move from `currentTier` to the next tier
    // in `category`. Returns -1 if already at max tier.
    static int upgradeCost(TuningCategory category, int currentTier);

    // Attempts to buy the next tier of `category` for `region`, spending
    // reputation from `progression`. Returns true on success.
    static bool purchaseUpgrade(TuningState &state, TuningCategory category,
        ProgressionState &progression, RegionId region);

    // Recomputes `car`'s tuning stats from scratch (stock baseline) and
    // applies every tier's modifiers from `state`. Safe to call anytime,
    // e.g. right after a purchase, to make the change take effect
    // immediately on the live Car instance.
    static void apply(const TuningState &state, Car &car);
};

} // namespace racer::world

#endif /* !TUNING_SYSTEM_HPP_ */
