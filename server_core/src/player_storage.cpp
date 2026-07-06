#include "server_core/player_storage.h"

#include <cstdint>
#include <filesystem>
#include <fstream>

namespace server_core {

namespace {
constexpr uint32_t kPlayerFileMagic = 0x564F5850; // 'V','O','X','P'
constexpr uint8_t kPlayerFileVersion = 1;
} // namespace

PlayerStorage::PlayerStorage(std::string rootDir) : rootDir_(std::move(rootDir)) {
    std::filesystem::create_directories(std::filesystem::path(rootDir_) / "players");
}

std::string PlayerStorage::PathFor(const std::string& playerName) const {
    return (std::filesystem::path(rootDir_) / "players" / (playerName + ".dat")).string();
}

bool PlayerStorage::Load(const std::string& playerName, PlayerSaveData& outData) const {
    std::ifstream file(PathFor(playerName), std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t magic = 0;
    uint8_t version = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (!file || magic != kPlayerFileMagic || version != kPlayerFileVersion) return false;

    file.read(reinterpret_cast<char*>(&outData.posX), sizeof(outData.posX));
    file.read(reinterpret_cast<char*>(&outData.posY), sizeof(outData.posY));
    file.read(reinterpret_cast<char*>(&outData.posZ), sizeof(outData.posZ));
    file.read(reinterpret_cast<char*>(&outData.health), sizeof(outData.health));
    file.read(reinterpret_cast<char*>(&outData.hunger), sizeof(outData.hunger));
    for (auto& slot : outData.inventory) {
        file.read(reinterpret_cast<char*>(&slot.blockId), sizeof(slot.blockId));
        file.read(reinterpret_cast<char*>(&slot.count), sizeof(slot.count));
    }

    return static_cast<bool>(file);
}

void PlayerStorage::Save(const std::string& playerName, const PlayerSaveData& data) const {
    std::ofstream file(PathFor(playerName), std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return;

    uint32_t magic = kPlayerFileMagic;
    uint8_t version = kPlayerFileVersion;
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&data.posX), sizeof(data.posX));
    file.write(reinterpret_cast<const char*>(&data.posY), sizeof(data.posY));
    file.write(reinterpret_cast<const char*>(&data.posZ), sizeof(data.posZ));
    file.write(reinterpret_cast<const char*>(&data.health), sizeof(data.health));
    file.write(reinterpret_cast<const char*>(&data.hunger), sizeof(data.hunger));
    for (const auto& slot : data.inventory) {
        file.write(reinterpret_cast<const char*>(&slot.blockId), sizeof(slot.blockId));
        file.write(reinterpret_cast<const char*>(&slot.count), sizeof(slot.count));
    }
}

} // namespace server_core
