/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Open world mission tracking
*/

#ifndef MISSION_SYSTEM_HPP_
#define MISSION_SYSTEM_HPP_

#include <vector>

#include "World/Aurelia/AureliaData.hpp"
#include "World/Sim/ProgressionState.hpp"

namespace racer::world {

class MissionSystem {
public:
    explicit MissionSystem(ProgressionState &progression);

    void update(float dt, float playerX, float playerZ, float playerSpeed);
    bool tryStartMission(int missionIndex);
    void completeActiveMission();

    int activeMissionIndex() const { return active_; }
    float activeTimer() const { return activeTimer_; }
    const std::vector<MissionState> &states() const { return states_; }

private:
    ProgressionState &progression_;
    std::vector<MissionState> states_;
    int active_ = -1;
    float activeTimer_ = 0.0f;
};

} // namespace racer::world

#endif /* !MISSION_SYSTEM_HPP_ */
