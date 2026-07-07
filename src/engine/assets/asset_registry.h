#pragma once

// Registre d'assets centralise : cache par chemin normalise, refcount simple,
// placeholders magenta pour tout asset manquant (visibles immediatement).
// Contrainte : thread principal uniquement (contexte OpenGL requis).

#include "raylib.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace racer::engine {

// Infos PBR extraites d'un materiau raylib (renseignees par l'import GLTF/GLB).
struct PbrMaterialInfo {
    Color albedoColor{255, 255, 255, 255}; // facteur baseColor
    Color emissiveColor{0, 0, 0, 255};     // facteur emissif
    float metallic = 0.0f;                 // MATERIAL_MAP_METALNESS .value
    float roughness = 1.0f;                // MATERIAL_MAP_ROUGHNESS .value
    bool hasAlbedoMap = false;             // texture baseColor presente
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;  // texture combinee GLTF (metal+rough)
    bool hasEmissiveMap = false;
    bool hasOcclusionMap = false;
};

// Modele en cache, detenu par AssetRegistry. La reference retournee reste
// stable tant que l'asset n'est pas decharge (UnloadUnused / UnloadAll).
class ModelAsset {
public:
    Model& Get() { return model_; }
    const Model& Get() const { return model_; }

    int MeshCount() const { return model_.meshCount; }
    int MaterialCount() const { return model_.materialCount; }

    // Une entree par materiau, index aligne sur model.materials.
    const std::vector<PbrMaterialInfo>& PbrInfos() const { return pbrInfos_; }

    bool IsPlaceholder() const { return placeholder_; }
    const std::string& Path() const { return path_; }

    // Refcount cooperatif : LoadModelAsset() fait un Acquire() implicite.
    void Acquire() { ++refCount_; }
    void Release();
    int RefCount() const { return refCount_; }

private:
    friend class AssetRegistry;

    Model model_{};
    std::vector<PbrMaterialInfo> pbrInfos_;
    std::string path_; // chemin normalise (separateurs '/')
    bool placeholder_ = false;
    int refCount_ = 0;
};

// Texture en cache (mipmaps + filtrage trilineaire appliques au chargement).
class TextureAsset {
public:
    Texture2D& Get() { return texture_; }
    const Texture2D& Get() const { return texture_; }

    bool IsPlaceholder() const { return placeholder_; }
    const std::string& Path() const { return path_; }

    void Acquire() { ++refCount_; }
    void Release();
    int RefCount() const { return refCount_; }

private:
    friend class AssetRegistry;

    Texture2D texture_{};
    std::string path_;
    bool placeholder_ = false;
    int refCount_ = 0;
};

// Cache centralise de modeles et textures.
class AssetRegistry {
public:
    AssetRegistry() = default;
    ~AssetRegistry();

    AssetRegistry(const AssetRegistry&) = delete;
    AssetRegistry& operator=(const AssetRegistry&) = delete;

    // Charge (ou recupere du cache) un modele ; Acquire() implicite.
    // Manquant/corrompu -> TraceLog(LOG_WARNING) + cube magenta placeholder.
    ModelAsset& LoadModelAsset(const std::string& path);

    // Charge (ou recupere du cache) une texture ; Acquire() implicite.
    // Manquante -> TraceLog(LOG_WARNING) + damier magenta placeholder.
    TextureAsset& LoadTextureAsset(const std::string& path);

    // Decharge les assets dont le refcount est retombe a zero.
    void UnloadUnused();

    // Decharge tout (a appeler avant CloseWindow()).
    void UnloadAll();

    int ModelCount() const { return static_cast<int>(models_.size()); }
    int TextureCount() const { return static_cast<int>(textures_.size()); }

private:
    std::unordered_map<std::string, std::unique_ptr<ModelAsset>> models_;
    std::unordered_map<std::string, std::unique_ptr<TextureAsset>> textures_;
};

} // namespace racer::engine
