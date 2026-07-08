/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Regional reputation and unlocks
*/

#ifndef PROGRESSION_STATE_HPP_
#define PROGRESSION_STATE_HPP_

#include <array>
#include <cstdint>

#include "World/Aurelia/AureliaTypes.hpp"

namespace racer::world {

class ProgressionState {
public:
    static constexpr int kRegionCount = static_cast<int>(RegionId::COUNT);

    int reputation(RegionId region) const;
    void addReputation(RegionId region, int amount);
    bool spendReputation(RegionId region, int amount);
    bool garageUnlocked(RegionId region) const;
    bool cendresCircuitUnlocked() const;
    int loreCollected() const { return loreCollected_; }
    void collectLore(int index);
    // Per-index query used by the pause-menu encyclopedia to know which of
    // the 20 lore fragments have already been picked up.
    bool loreCollectedAt(int index) const;

    // Raw accessors used by SaveSystem to persist/restore state without
    // going through the gameplay side-effects of addReputation()/
    // collectLore() (e.g. reputation bonus for lore pickups).
    void setReputation(RegionId region, int amount);
    std::uint8_t garageMask() const { return garageMask_; }
    void setGarageMask(std::uint8_t mask) { garageMask_ = mask; }
    std::uint32_t loreMask() const { return loreMask_; }
    void setLoreMask(std::uint32_t mask);

private:
    std::array<int, static_cast<size_t>(RegionId::COUNT)> rep_{};
    std::uint8_t garageMask_ = 0x01;
    std::uint32_t loreMask_ = 0;
    int loreCollected_ = 0;
};

} // namespace racer::world

#endif /* !PROGRESSION_STATE_HPP_ */
