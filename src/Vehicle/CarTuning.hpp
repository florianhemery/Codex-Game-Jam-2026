/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Vehicle physics tuning parameters
*/

#ifndef CAR_TUNING_HPP_
#define CAR_TUNING_HPP_

namespace racer {

struct CarTuning {
    float maxSpeed = 28.0f;
    float acceleration = 14.0f;
    float braking = 24.0f;
    float turnRate = 2.6f;
    float gripNormal = 6.0f;
    float gripDrift = 0.8f;
    float dragCoeff = 0.35f;
    float nitroBoost = 12.0f;
    float nitroMaxSpeedBonus = 10.0f;
    float nitroCapacity = 3.0f;
    float nitroRegenPerSecond = 0.4f;
};

} // namespace racer

#endif /* !CAR_TUNING_HPP_ */
