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
    int reputation(RegionId region) const;
    void addReputation(RegionId region, int amount);
    bool garageUnlocked(RegionId region) const;
    bool cendresCircuitUnlocked() const;
    int loreCollected() const { return loreCollected_; }
    void collectLore(int index);

private:
    std::array<int, static_cast<size_t>(RegionId::COUNT)> rep_{};
    std::uint8_t garageMask_ = 0x01;
    std::uint32_t loreMask_ = 0;
    int loreCollected_ = 0;
};

} // namespace racer::world

#endif /* !PROGRESSION_STATE_HPP_ */
