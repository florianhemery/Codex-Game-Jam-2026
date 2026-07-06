#pragma once

#include <cstdint>
#include <variant>

namespace common::messages {

enum class UnreliableMsgType : uint8_t {
    PlayerInput,
};

// Jour 2 : porte la position courante du joueur (pas encore des deltas d'intention),
// utilisee par server_core uniquement pour savoir autour de quel point streamer les
// chunks. Cette simplification est deliberee : sans SocketTransport, il n'y a pas
// encore d'enjeu anti-triche a valider un mouvement plutot qu'un etat.
struct PlayerInputMsg {
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
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
