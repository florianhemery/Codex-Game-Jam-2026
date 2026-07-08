/*
** EPITECH PROJECT, 2026
** racer
** File description:
** MissionSystem implementation
*/

#include "World/Sim/MissionSystem.hpp"

#include <cmath>

namespace racer::world {

MissionSystem::MissionSystem(ProgressionState &progression)
    : progression_(progression)
{
    states_.assign(AureliaData::missions().size(), MissionState::AVAILABLE);
}

bool MissionSystem::tryStartMission(int missionIndex)
{
    if (active_ >= 0) {
        return false;
    }
    if (missionIndex < 0
        || missionIndex >= static_cast<int>(AureliaData::missions().size())) {
        return false;
    }
    if (states_[static_cast<size_t>(missionIndex)] == MissionState::COMPLETED) {
        return false;
    }
    active_ = missionIndex;
    activeTimer_ = AureliaData::missions()[static_cast<size_t>(missionIndex)]
        .targetTime;
    states_[static_cast<size_t>(missionIndex)] = MissionState::ACTIVE;
    return true;
}

void MissionSystem::completeActiveMission()
{
    if (active_ < 0) {
        return;
    }
    const MissionDef &def =
        AureliaData::missions()[static_cast<size_t>(active_)];
    progression_.addReputation(def.region, def.rewardRep);
    states_[static_cast<size_t>(active_)] = MissionState::COMPLETED;
    active_ = -1;
    activeTimer_ = 0.0f;
}

void MissionSystem::update(float dt, float playerX, float playerZ,
    float playerSpeed)
{
    if (active_ < 0) {
        return;
    }
    activeTimer_ -= dt;
    const MissionDef &def =
        AureliaData::missions()[static_cast<size_t>(active_)];

    bool success = false;
    switch (def.kind) {
    case MissionKind::CONVOY:
    case MissionKind::DELIVERY:
        success = playerSpeed > 8.0f && activeTimer_ > 0.0f;
        break;
    case MissionKind::GHOST_CHASE:
    case MissionKind::TIME_TRIAL:
        success = activeTimer_ > 0.0f && playerSpeed > 12.0f;
        break;
    case MissionKind::FOG_ESCAPE:
        success = playerZ < -80.0f && activeTimer_ > 0.0f;
        break;
    case MissionKind::CALDERA_CLIMB:
        success = playerZ > 160.0f && activeTimer_ > 0.0f;
        break;
    }

    if (success && activeTimer_ < def.targetTime * 0.35f) {
        completeActiveMission();
    } else if (activeTimer_ <= 0.0f) {
        states_[static_cast<size_t>(active_)] = MissionState::AVAILABLE;
        active_ = -1;
    }
}

} // namespace racer::world
