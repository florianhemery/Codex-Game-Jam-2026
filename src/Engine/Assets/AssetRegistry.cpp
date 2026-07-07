/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Asset registry implementation
*/

#include "Engine/Assets/AssetRegistry.hpp"

#include <filesystem>
#include <utility>

namespace racer::engine {

class AssetRegistryDetail {
public:
    static std::string normalizePath(const std::string &path);
    static Model makePlaceholderModel();
    static Texture2D makePlaceholderTexture();
    static PbrMaterialInfo extractPbrInfo(const Material &mat);
    static void populatePbrInfos(ModelAsset &asset);
    static void applyTextureDefaults(Texture2D &texture);
    static bool tryLoadModelFromDisk(
        const std::string &key, ModelAsset &asset);
    static bool tryLoadTextureFromDisk(
        const std::string &key, TextureAsset &asset);
    static void assignModelPlaceholder(
        ModelAsset &asset, const std::string &key);
    static void assignTexturePlaceholder(
        TextureAsset &asset, const std::string &key);
    static void unloadZeroRefModels(
        std::unordered_map<std::string, std::unique_ptr<ModelAsset>> &models);
    static void unloadZeroRefTextures(
        std::unordered_map<std::string,
            std::unique_ptr<TextureAsset>> &textures);
};

std::string AssetRegistryDetail::normalizePath(const std::string &path)
{
    return std::filesystem::path(path).lexically_normal().generic_string();
}

Model AssetRegistryDetail::makePlaceholderModel()
{
    Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    model.materials[0].maps[MATERIAL_MAP_ALBEDO].color = MAGENTA;
    return model;
}

Texture2D AssetRegistryDetail::makePlaceholderTexture()
{
    Image image = GenImageChecked(64, 64, 8, 8, MAGENTA, BLACK);
    Texture2D texture = LoadTextureFromImage(image);

    UnloadImage(image);
    return texture;
}

PbrMaterialInfo AssetRegistryDetail::extractPbrInfo(const Material &mat)
{
    PbrMaterialInfo info{};

    info.albedoColor = mat.maps[MATERIAL_MAP_ALBEDO].color;
    info.emissiveColor = mat.maps[MATERIAL_MAP_EMISSION].color;
    info.metallic = mat.maps[MATERIAL_MAP_METALNESS].value;
    info.roughness = mat.maps[MATERIAL_MAP_ROUGHNESS].value;
    info.hasAlbedoMap = (mat.maps[MATERIAL_MAP_ALBEDO].texture.id != 0);
    info.hasNormalMap = (mat.maps[MATERIAL_MAP_NORMAL].texture.id != 0);
    info.hasMetallicRoughnessMap =
        (mat.maps[MATERIAL_MAP_ROUGHNESS].texture.id != 0);
    info.hasEmissiveMap = (mat.maps[MATERIAL_MAP_EMISSION].texture.id != 0);
    info.hasOcclusionMap =
        (mat.maps[MATERIAL_MAP_OCCLUSION].texture.id != 0);
    return info;
}

void AssetRegistryDetail::populatePbrInfos(ModelAsset &asset)
{
    asset.pbrInfos_.reserve(
        static_cast<size_t>(asset.model_.materialCount));
    for (int i = 0; i < asset.model_.materialCount; ++i) {
        asset.pbrInfos_.push_back(
            extractPbrInfo(asset.model_.materials[i]));
    }
}

void AssetRegistryDetail::applyTextureDefaults(Texture2D &texture)
{
    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
}

bool AssetRegistryDetail::tryLoadModelFromDisk(
    const std::string &key, ModelAsset &asset)
{
    bool loaded = false;

    if (FileExists(key.c_str())) {
        Model model = LoadModel(key.c_str());

        if (IsModelValid(model)) {
            asset.model_ = model;
            loaded = true;
        } else {
            UnloadModel(model);
        }
    }
    return loaded;
}

bool AssetRegistryDetail::tryLoadTextureFromDisk(
    const std::string &key, TextureAsset &asset)
{
    bool loaded = false;

    if (FileExists(key.c_str())) {
        Texture2D texture = LoadTexture(key.c_str());

        if (IsTextureValid(texture)) {
            asset.texture_ = texture;
            loaded = true;
        }
    }
    return loaded;
}

void AssetRegistryDetail::assignModelPlaceholder(
    ModelAsset &asset, const std::string &key)
{
    TraceLog(
        LOG_WARNING,
        "ASSETS: [%s] modele introuvable ou invalide, cube magenta utilise",
        key.c_str());
    asset.model_ = makePlaceholderModel();
    asset.placeholder_ = true;
}

void AssetRegistryDetail::assignTexturePlaceholder(
    TextureAsset &asset, const std::string &key)
{
    TraceLog(
        LOG_WARNING,
        "ASSETS: [%s] texture introuvable ou invalide, damier magenta utilise",
        key.c_str());
    asset.texture_ = makePlaceholderTexture();
    asset.placeholder_ = true;
}

void ModelAsset::release()
{
    if (refCount_ > 0)
        --refCount_;
}

void TextureAsset::release()
{
    if (refCount_ > 0)
        --refCount_;
}

AssetRegistry::~AssetRegistry()
{
    unloadAll();
}

ModelAsset *AssetRegistry::findAndAcquireModel(const std::string &key)
{
    auto it = models_.find(key);

    if (it == models_.end())
        return nullptr;
    it->second->acquire();
    return it->second.get();
}

ModelAsset &AssetRegistry::insertNewModel(const std::string &key)
{
    auto asset = std::make_unique<ModelAsset>();

    asset->path_ = key;
    if (!AssetRegistryDetail::tryLoadModelFromDisk(key, *asset))
        AssetRegistryDetail::assignModelPlaceholder(*asset, key);
    AssetRegistryDetail::populatePbrInfos(*asset);
    asset->acquire();
    ModelAsset *result = asset.get();
    models_.emplace(key, std::move(asset));
    return *result;
}

ModelAsset &AssetRegistry::loadModelAsset(const std::string &path)
{
    const std::string key = AssetRegistryDetail::normalizePath(path);

    if (ModelAsset *existing = findAndAcquireModel(key))
        return *existing;
    return insertNewModel(key);
}

TextureAsset *AssetRegistry::findAndAcquireTexture(const std::string &key)
{
    auto it = textures_.find(key);

    if (it == textures_.end())
        return nullptr;
    it->second->acquire();
    return it->second.get();
}

TextureAsset &AssetRegistry::insertNewTexture(const std::string &key)
{
    auto asset = std::make_unique<TextureAsset>();

    asset->path_ = key;
    if (!AssetRegistryDetail::tryLoadTextureFromDisk(key, *asset))
        AssetRegistryDetail::assignTexturePlaceholder(*asset, key);
    AssetRegistryDetail::applyTextureDefaults(asset->texture_);
    asset->acquire();
    TextureAsset *result = asset.get();
    textures_.emplace(key, std::move(asset));
    return *result;
}

TextureAsset &AssetRegistry::loadTextureAsset(const std::string &path)
{
    const std::string key = AssetRegistryDetail::normalizePath(path);

    if (TextureAsset *existing = findAndAcquireTexture(key))
        return *existing;
    return insertNewTexture(key);
}

void AssetRegistryDetail::unloadZeroRefModels(
    std::unordered_map<std::string, std::unique_ptr<ModelAsset>> &models)
{
    for (auto it = models.begin(); it != models.end();) {
        if (it->second->refCount() <= 0) {
            UnloadModel(it->second->model_);
            TraceLog(
                LOG_INFO,
                "ASSETS: [%s] modele decharge",
                it->first.c_str());
            it = models.erase(it);
        } else {
            ++it;
        }
    }
}

void AssetRegistryDetail::unloadZeroRefTextures(
    std::unordered_map<std::string, std::unique_ptr<TextureAsset>> &textures)
{
    for (auto it = textures.begin(); it != textures.end();) {
        if (it->second->refCount() <= 0) {
            UnloadTexture(it->second->texture_);
            TraceLog(
                LOG_INFO,
                "ASSETS: [%s] texture dechargee",
                it->first.c_str());
            it = textures.erase(it);
        } else {
            ++it;
        }
    }
}

void AssetRegistry::unloadUnused()
{
    AssetRegistryDetail::unloadZeroRefModels(models_);
    AssetRegistryDetail::unloadZeroRefTextures(textures_);
}

void AssetRegistry::unloadAll()
{
    const bool gpuReady = IsWindowReady();

    for (auto &entry : models_) {
        if (gpuReady)
            UnloadModel(entry.second->model_);
    }
    for (auto &entry : textures_) {
        if (gpuReady)
            UnloadTexture(entry.second->texture_);
    }
    models_.clear();
    textures_.clear();
}

} // namespace racer::engine
