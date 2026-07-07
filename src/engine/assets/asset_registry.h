/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Centralized model and texture asset cache
*/

#ifndef ASSET_REGISTRY_H_
#define ASSET_REGISTRY_H_

#include "raylib.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace racer::engine {

struct PbrMaterialInfo {
    Color albedoColor{255, 255, 255, 255};
    Color emissiveColor{0, 0, 0, 255};
    float metallic = 0.0f;
    float roughness = 1.0f;
    bool hasAlbedoMap = false;
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;
    bool hasEmissiveMap = false;
    bool hasOcclusionMap = false;
};

class ModelAsset {
public:
    Model &Get()
    {
        return model_;
    }

    const Model &Get() const
    {
        return model_;
    }

    int MeshCount() const
    {
        return model_.meshCount;
    }

    int MaterialCount() const
    {
        return model_.materialCount;
    }

    const std::vector<PbrMaterialInfo> &PbrInfos() const
    {
        return pbrInfos_;
    }

    bool IsPlaceholder() const
    {
        return placeholder_;
    }

    const std::string &Path() const
    {
        return path_;
    }

    void Acquire()
    {
        ++refCount_;
    }

    void Release();
    int RefCount() const
    {
        return refCount_;
    }

private:
    friend class AssetRegistry;

    Model model_{};
    std::vector<PbrMaterialInfo> pbrInfos_;
    std::string path_;
    bool placeholder_ = false;
    int refCount_ = 0;
};

class TextureAsset {
public:
    Texture2D &Get()
    {
        return texture_;
    }

    const Texture2D &Get() const
    {
        return texture_;
    }

    bool IsPlaceholder() const
    {
        return placeholder_;
    }

    const std::string &Path() const
    {
        return path_;
    }

    void Acquire()
    {
        ++refCount_;
    }

    void Release();
    int RefCount() const
    {
        return refCount_;
    }

private:
    friend class AssetRegistry;

    Texture2D texture_{};
    std::string path_;
    bool placeholder_ = false;
    int refCount_ = 0;
};

class AssetRegistry {
public:
    AssetRegistry() = default;
    ~AssetRegistry();

    AssetRegistry(const AssetRegistry &) = delete;
    AssetRegistry &operator=(const AssetRegistry &) = delete;

    ModelAsset &LoadModelAsset(const std::string &path);
    TextureAsset &LoadTextureAsset(const std::string &path);
    void UnloadUnused();
    void UnloadAll();

    int ModelCount() const
    {
        return static_cast<int>(models_.size());
    }

    int TextureCount() const
    {
        return static_cast<int>(textures_.size());
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ModelAsset>> models_;
    std::unordered_map<std::string, std::unique_ptr<TextureAsset>> textures_;
};

} // namespace racer::engine

#endif /* !ASSET_REGISTRY_H_ */
