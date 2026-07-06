#pragma once

#include <cstdint>
#include <string>

#include "common/messages/reliable_messages.h"
#include "common/messages/unreliable_messages.h"

// API publique de server_core. Pure logique de jeu : pas de socket, pas de raylib.
// Un "client" ici est identifie par un ClientId opaque, jamais par une adresse
// reseau -- meme en solo, le joueur local passe par connect_client() comme
// n'importe quel futur client reseau le fera.

namespace server_core {

using ClientId = uint32_t;

struct ServerCore;

struct ServerConfig {
    std::string worldSaveDir;
    uint32_t seed = 0;
};

ServerCore* create(const ServerConfig& cfg);
void destroy(ServerCore* sc);

ClientId connect_client(ServerCore* sc, const std::string& playerName);
void disconnect_client(ServerCore* sc, ClientId id);

void submit_reliable(ServerCore* sc, ClientId from, const common::messages::ReliableMessage& msg);
void submit_unreliable(ServerCore* sc, ClientId from, const common::messages::UnreliableMessage& msg);

// Accumule dt en interne et execute des pas de simulation a cadence fixe.
void tick(ServerCore* sc, float dt);

bool poll_outgoing_reliable(ServerCore* sc, ClientId to, common::messages::ReliableMessage& out);
bool poll_outgoing_unreliable(ServerCore* sc, ClientId to, common::messages::UnreliableMessage& out);

} // namespace server_core
