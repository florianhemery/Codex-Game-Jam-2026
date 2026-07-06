#pragma once

#include <cstdint>

namespace server_core {

enum class MobType : uint8_t {
    Pig,
    Zombie,
};

struct Entity {
    uint32_t id = 0;
    MobType type = MobType::Pig;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float velY = 0.0f;
    float yaw = 0.0f;
    float wanderTimer = 0.0f;
};

} // namespace server_core
