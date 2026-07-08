/*
** EPITECH PROJECT, 2026
** racer
** File description:
** ProgressionState implementation
*/

#include "World/Sim/ProgressionState.hpp"

#include <algorithm>

namespace racer::world {

int ProgressionState::reputation(RegionId region) const
{
    size_t i = static_cast<size_t>(region);
    if (i >= rep_.size()) {
        return 0;
    }
    return rep_[i];
}

void ProgressionState::addReputation(RegionId region, int amount)
{
    size_t i = static_cast<size_t>(region);
    if (i >= rep_.size()) {
        return;
    }
    rep_[i] = std::min(100, rep_[i] + amount);
    if (rep_[i] >= 25) {
        garageMask_ |= static_cast<std::uint8_t>(1u << i);
    }
}

bool ProgressionState::spendReputation(RegionId region, int amount)
{
    size_t i = static_cast<size_t>(region);
    if (i >= rep_.size() || amount <= 0 || rep_[i] < amount) {
        return false;
    }
    rep_[i] -= amount;
    return true;
}

bool ProgressionState::garageUnlocked(RegionId region) const
{
    size_t i = static_cast<size_t>(region);
    if (i >= 8) {
        return false;
    }
    return (garageMask_ & static_cast<std::uint8_t>(1u << i)) != 0;
}

bool ProgressionState::cendresCircuitUnlocked() const
{
    return reputation(RegionId::VOLCANO) >= 50;
}

void ProgressionState::setReputation(RegionId region, int amount)
{
    size_t i = static_cast<size_t>(region);
    if (i >= rep_.size()) {
        return;
    }
    rep_[i] = amount;
}

void ProgressionState::setLoreMask(std::uint32_t mask)
{
    loreMask_ = mask;
    int count = 0;
    for (std::uint32_t bit = mask; bit != 0; bit &= bit - 1) {
        ++count;
    }
    loreCollected_ = count;
}

void ProgressionState::collectLore(int index)
{
    if (index < 0 || index >= 20) {
        return;
    }
    std::uint32_t bit = 1u << static_cast<unsigned>(index);
    if ((loreMask_ & bit) != 0) {
        return;
    }
    loreMask_ |= bit;
    ++loreCollected_;
    addReputation(RegionId::MARINA, 2);
}

bool ProgressionState::loreCollectedAt(int index) const
{
    if (index < 0 || index >= 20) {
        return false;
    }
    return (loreMask_ & (1u << static_cast<unsigned>(index))) != 0;
}

} // namespace racer::world
