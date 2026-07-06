#pragma once

#include <cstdint>
#include <variant>

namespace common::messages {

enum class UnreliableMsgType : uint8_t {
    PlayerInput,
};

struct PlayerInputMsg {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
    uint8_t actionFlags = 0;
    uint32_t tick = 0;
};

struct UnreliableMessage {
    UnreliableMsgType type;
    std::variant<PlayerInputMsg> payload;
};

} // namespace common::messages
