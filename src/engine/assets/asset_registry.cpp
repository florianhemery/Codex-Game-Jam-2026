// Registre d'assets : charge modeles/textures via raylib, cache par chemin
// normalise, refcount cooperatif, placeholders magenta si asset manquant.

#include "engine/assets/asset_registry.h"

#include <filesystem>
#include <utility>

namespace racer::engine {

namespace {

// Normalise le chemin (separateurs '/', "./" et "a/../b" reduits) pour
// servir de cle de cache stable quelle que soit l'ecriture d'origine.
std::string NormalizePath(const std::string& path)
{
    return std::filesystem::path(path).lexically_normal().generic_string();
}

// Cube magenta unitaire : placeholder immediatement visible a l'ecran.
Model MakePlaceholderModel()
{
    Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].color = MAGENTA;
    return model;
}

// Damier magenta/noir 64x64 genere en code : placeholder de texture.
Texture2D MakePlaceholderTexture()
{
    Image image = GenImageChecked(64, 64, 8, 8, MAGENTA, BLACK);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

// Recopie les infos PBR que raylib a renseignees dans les maps du materiau
// (l'import GLTF remplit albedo/emissive/metallic/roughness et les textures).
PbrMaterialInfo ExtractPbrInfo(const Material& mat)
{
    PbrMaterialInfo info{};
    info.albedoColor = mat.maps[MATERIAL_MAP_ALBEDO].color;
    info.emissiveColor = mat.maps[MATERIAL_MAP_EMISSION].color;
    info.metallic = mat.maps[MATERIAL_MAP_METALNESS].value;
    info.roughness = mat.maps[MATERIAL_MAP_ROUGHNESS].value;
    info.hasAlbedoMap = (mat.maps[MATERIAL_MAP_ALBEDO].texture.id != 0);
    info.hasNormalMap = (mat.maps[MATERIAL_MAP_NORMAL].texture.id != 0);
    info.hasMetallicRoughnessMap = (mat.maps[MATERIAL_MAP_ROUGHNESS].texture.id != 0);
    info.hasEmissiveMap = (mat.maps[MATERIAL_MAP_EMISSION].texture.id != 0);
    info.hasOcclusionMap = (mat.maps[MATERIAL_MAP_OCCLUSION].texture.id != 0);
    return info;
}

} // namespace

void ModelAsset::Release()
{
    if (refCount_ > 0) --refCount_;
}

void TextureAsset::Release()
{
    if (refCount_ > 0) --refCount_;
}

AssetRegistry::~AssetRegistry()
{
    UnloadAll();
}

ModelAsset& AssetRegistry::LoadModelAsset(const std::string& path)
{
    const std::string key = NormalizePath(path);

    if (auto it = models_.find(key); it != models_.end()) {
        it->second->Acquire();
        return *it->second;
    }

    auto asset = std::make_unique<ModelAsset>();
    asset->path_ = key;

    bool loaded = false;
    if (FileExists(key.c_str())) {
        Model model = LoadModel(key.c_str());
        if (IsModelValid(model)) {
            asset->model_ = model;
            loaded = true;
        } else {
            UnloadModel(model); // libere les allocations partielles eventuelles
        }
    }

    if (!loaded) {
        TraceLog(LOG_WARNING, "ASSETS: [%s] modele introuvable ou invalide, cube magenta utilise", key.c_str());
        asset->model_ = MakePlaceholderModel();
        asset->placeholder_ = true;
    }

    asset->pbrInfos_.reserve(static_cast<size_t>(asset->model_.materialCount));
    for (int i = 0; i < asset->model_.materialCount; ++i) {
        asset->pbrInfos_.push_back(ExtractPbrInfo(asset->model_.materials[i]));
    }

    asset->Acquire();
    ModelAsset& ref = *asset;
    models_.emplace(key, std::move(asset));
    return ref;
}

TextureAsset& AssetRegistry::LoadTextureAsset(const std::string& path)
{
    const std::string key = NormalizePath(path);

    if (auto it = textures_.find(key); it != textures_.end()) {
        it->second->Acquire();
        return *it->second;
    }

    auto asset = std::make_unique<TextureAsset>();
    asset->path_ = key;

    bool loaded = false;
    if (FileExists(key.c_str())) {
        Texture2D texture = LoadTexture(key.c_str());
        if (IsTextureValid(texture)) {
            asset->texture_ = texture;
            loaded = true;
        }
    }

    if (!loaded) {
        TraceLog(LOG_WARNING, "ASSETS: [%s] texture introuvable ou invalide, damier magenta utilise", key.c_str());
        asset->texture_ = MakePlaceholderTexture();
        asset->placeholder_ = true;
    }

    // Qualite par defaut : mipmaps + filtrage trilineaire systematiques.
    GenTextureMipmaps(&asset->texture_);
    SetTextureFilter(asset->texture_, TEXTURE_FILTER_TRILINEAR);

    asset->Acquire();
    TextureAsset& ref = *asset;
    textures_.emplace(key, std::move(asset));
    return ref;
}

void AssetRegistry::UnloadUnused()
{
    for (auto it = models_.begin(); it != models_.end();) {
        if (it->second->RefCount() <= 0) {
            UnloadModel(it->second->model_);
            TraceLog(LOG_INFO, "ASSETS: [%s] modele decharge", it->first.c_str());
            it = models_.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = textures_.begin(); it != textures_.end();) {
        if (it->second->RefCount() <= 0) {
            UnloadTexture(it->second->texture_);
            TraceLog(LOG_INFO, "ASSETS: [%s] texture dechargee", it->first.c_str());
            it = textures_.erase(it);
        } else {
            ++it;
        }
    }
}

void AssetRegistry::UnloadAll()
{
    // Contexte GL deja ferme -> on ne touche plus au GPU (fin de process,
    // l'OS recupere la memoire) ; sinon dechargement propre.
    const bool gpuReady = IsWindowReady();

    for (auto& entry : models_) {
        if (gpuReady) UnloadModel(entry.second->model_);
    }
    for (auto& entry : textures_) {
        if (gpuReady) UnloadTexture(entry.second->texture_);
    }

    models_.clear();
    textures_.clear();
}

} // namespace racer::engine
