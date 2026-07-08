/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Player vehicle driving on streamed terrain
*/

#ifndef PLAYER_DRIVE_SYSTEM_HPP_
#define PLAYER_DRIVE_SYSTEM_HPP_

#include "Vehicle/Car.hpp"
#include "Vehicle/CarInput.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace racer::world {

class PlayerDriveSystem {
public:
    static constexpr float kWheelRadius = 0.35f;

    void reset(Car &car, float worldX, float worldZ, float heading);
    void update(Car &car, const CarInput &input, float steerSmoothed,
        float dt, ChunkStreamer &streamer, float &wheelSpinOut);

private:
    static void applySurface(Car &car, SurfaceKind surface, float dt);
};

} // namespace racer::world

#endif /* !PLAYER_DRIVE_SYSTEM_HPP_ */
