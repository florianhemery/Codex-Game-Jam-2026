#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "server_core/inventory.h"

namespace server_core {

struct PlayerSaveData {
    float posX = 8.0f;
    float posY = 40.0f;
    float posZ = 8.0f;
    uint16_t health = 100;
    uint16_t hunger = 100;
    std::array<InventorySlot, common::messages::kInventorySlotCount> inventory{};
};

// Un fichier par joueur (world/players/{name}.dat), format binaire simple.
class PlayerStorage {
public:
    explicit PlayerStorage(std::string rootDir);

    bool Load(const std::string& playerName, PlayerSaveData& outData) const;
    void Save(const std::string& playerName, const PlayerSaveData& data) const;

private:
    std::string PathFor(const std::string& playerName) const;

    std::string rootDir_;
};

} // namespace server_core
