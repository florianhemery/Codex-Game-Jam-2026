#pragma once

#include <array>
#include <variant>

#include "common/world/chunk.h"

namespace common::messages {

enum class ReliableMsgType : uint8_t {
    ChunkData,
    ChunkUnload,
    BlockUpdate,
    BreakBlockRequest,
    PlaceBlockRequest,
    InventoryUpdate,
    SpawnState,
    HealthHungerUpdate,
};

struct ChunkDataMsg {
    common::world::ChunkCoord coord;
    std::array<uint8_t, common::world::CHUNK_BLOCK_COUNT> blocks;
};

struct ChunkUnloadMsg {
    common::world::ChunkCoord coord;
};

// Diffuse par le serveur a tous les clients qui ont ce chunk charge --
// resultat authoritaire, jamais emis directement par un client.
struct BlockUpdateMsg {
    common::world::ChunkCoord coord;
    uint8_t lx, ly, lz;
    uint8_t newBlockId;
};

// Client -> serveur : intention de casser un bloc. Le serveur valide et
// decide seul du resultat (cf. BlockUpdateMsg en retour).
struct BreakBlockRequestMsg {
    common::world::ChunkCoord coord;
    uint8_t lx, ly, lz;
};

// Client -> serveur : intention de poser le contenu du slot hotbar indique.
// Le serveur choisit le bloc pose (celui du slot), jamais celui envoye par
// le client -- evite de faire confiance a un client sur "quoi" poser.
struct PlaceBlockRequestMsg {
    common::world::ChunkCoord coord;
    uint8_t lx, ly, lz;
    uint8_t hotbarSlot;
};

constexpr int kInventorySlotCount = 36;

struct InventorySlotWire {
    uint8_t blockId = 0;
    uint16_t count = 0;
};

struct InventoryUpdateMsg {
    std::array<InventorySlotWire, kInventorySlotCount> slots;
};

// Envoye juste apres connect_client, avant tout ChunkData : position/etat
// restaures depuis la sauvegarde disque, ou valeurs par defaut si premier lancement.
struct SpawnStateMsg {
    float posX = 8.0f;
    float posY = 40.0f;
    float posZ = 8.0f;
    uint16_t health = 100;
    uint16_t hunger = 100;
};

struct HealthHungerUpdateMsg {
    uint16_t health;
    uint16_t hunger;
};

using ReliableMessagePayload =
    std::variant<ChunkDataMsg, ChunkUnloadMsg, BlockUpdateMsg, BreakBlockRequestMsg, PlaceBlockRequestMsg,
                 InventoryUpdateMsg, SpawnStateMsg, HealthHungerUpdateMsg>;

struct ReliableMessage {
    ReliableMsgType type;
    ReliableMessagePayload payload;
};

} // namespace common::messages
