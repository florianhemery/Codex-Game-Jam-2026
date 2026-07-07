/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Centralized model and texture asset cache
*/

#ifndef ASSET_REGISTRY_HPP_
#define ASSET_REGISTRY_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/Assets/ModelAsset.hpp"
#include "Engine/Assets/TextureAsset.hpp"

namespace racer::engine {

class AssetRegistry {
public:
    AssetRegistry() = default;
    ~AssetRegistry();

    AssetRegistry(const AssetRegistry &) = delete;
    AssetRegistry &operator=(const AssetRegistry &) = delete;

    ModelAsset &loadModelAsset(const std::string &path);
    TextureAsset &loadTextureAsset(const std::string &path);
    void unloadUnused();
    void unloadAll();

    int modelCount() const
    {
        return static_cast<int>(models_.size());
    }

    int textureCount() const
    {
        return static_cast<int>(textures_.size());
    }

private:
    ModelAsset *findAndAcquireModel(const std::string &key);
    ModelAsset &insertNewModel(const std::string &key);
    TextureAsset *findAndAcquireTexture(const std::string &key);
    TextureAsset &insertNewTexture(const std::string &key);

    std::unordered_map<std::string, std::unique_ptr<ModelAsset>> models_;
    std::unordered_map<std::string, std::unique_ptr<TextureAsset>> textures_;
};

} // namespace racer::engine

#endif /* !ASSET_REGISTRY_HPP_ */
