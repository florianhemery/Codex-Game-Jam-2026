/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Loaded 3D model asset with PBR metadata
*/

#ifndef MODEL_ASSET_HPP_
#define MODEL_ASSET_HPP_

#include "raylib.h"

#include <string>
#include <vector>

namespace racer::engine {

class AssetRegistry;
class AssetRegistryDetail;

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
    Model &get()
    {
        return model_;
    }

    const Model &get() const
    {
        return model_;
    }

    int meshCount() const
    {
        return model_.meshCount;
    }

    int materialCount() const
    {
        return model_.materialCount;
    }

    const std::vector<PbrMaterialInfo> &pbrInfos() const
    {
        return pbrInfos_;
    }

    bool isPlaceholder() const
    {
        return placeholder_;
    }

    const std::string &path() const
    {
        return path_;
    }

    void acquire()
    {
        ++refCount_;
    }

    void release();
    int refCount() const
    {
        return refCount_;
    }

private:
    friend class AssetRegistry;

    friend class AssetRegistryDetail;

    Model model_{};
    std::vector<PbrMaterialInfo> pbrInfos_;
    std::string path_;
    bool placeholder_ = false;
    int refCount_ = 0;
};

} // namespace racer::engine

#endif /* !MODEL_ASSET_HPP_ */
