// Hot-reload de shaders : detection par mtime, rechargement securise
// (l'ancien shader reste actif si la nouvelle compilation echoue).

#include "engine/assets/shader_watcher.h"

#include "rlgl.h" // rlGetShaderIdDefault() : detection du fallback raylib

#include <utility>

namespace racer::engine {

namespace fs = std::filesystem;

namespace {

// mtime d'un fichier sans exception ; false si le fichier est inaccessible.
bool TryGetMTime(const std::string& path, fs::file_time_type& out)
{
    std::error_code ec;
    const fs::file_time_type time = fs::last_write_time(path, ec);
    if (ec) return false;
    out = time;
    return true;
}

const char* PathOrNull(const std::string& path)
{
    return path.empty() ? nullptr : path.c_str();
}

// Un chargement est reussi si le shader est valide ET n'est pas retombe sur
// le shader par defaut alors qu'on demandait des etages personnalises
// (raylib retourne l'id du shader par defaut quand la compilation echoue).
bool LoadSucceeded(const Shader& shader, bool wantsCustomStages)
{
    if (!IsShaderValid(shader)) return false;
    if (wantsCustomStages && shader.id == rlGetShaderIdDefault()) return false;
    return true;
}

// UnloadShader ignore le shader par defaut mais ne libere pas son tableau
// locs (alloue par LoadShader meme en cas de fallback) -> liberation manuelle.
void UnloadShaderSafe(Shader& shader)
{
    if (shader.id == 0) {
        shader = Shader{};
        return;
    }
    if (shader.id == rlGetShaderIdDefault()) {
        if (shader.locs != nullptr) MemFree(shader.locs);
    } else {
        UnloadShader(shader);
    }
    shader = Shader{};
}

} // namespace

ShaderWatcher::~ShaderWatcher()
{
    UnloadAll();
}

ShaderSlot& ShaderWatcher::RegisterShader(const std::string& name,
                                          const std::string& vsPath,
                                          const std::string& fsPath)
{
    if (auto it = slots_.find(name); it != slots_.end()) {
        TraceLog(LOG_WARNING, "SHADERS: [%s] deja enregistre, slot existant renvoye", name.c_str());
        return *it->second;
    }

    auto slot = std::make_unique<ShaderSlot>();
    slot->name_ = name;
    slot->vsPath_ = vsPath;
    slot->fsPath_ = fsPath;

    // mtimes initiaux (fichier absent -> sentinelle min, reload des apparition).
    if (!vsPath.empty() && !TryGetMTime(vsPath, slot->vsTime_)) {
        slot->vsTime_ = fs::file_time_type::min();
        TraceLog(LOG_WARNING, "SHADERS: [%s] vertex introuvable: %s", name.c_str(), vsPath.c_str());
    }
    if (!fsPath.empty() && !TryGetMTime(fsPath, slot->fsTime_)) {
        slot->fsTime_ = fs::file_time_type::min();
        TraceLog(LOG_WARNING, "SHADERS: [%s] fragment introuvable: %s", name.c_str(), fsPath.c_str());
    }

    const bool wantsCustom = !vsPath.empty() || !fsPath.empty();
    slot->shader_ = LoadShader(PathOrNull(vsPath), PathOrNull(fsPath));
    slot->valid_ = LoadSucceeded(slot->shader_, wantsCustom);
    if (!slot->valid_) {
        // Le shader retourne (defaut raylib) reste utilisable a l'ecran.
        TraceLog(LOG_WARNING, "SHADERS: [%s] compilation initiale echouee, shader par defaut actif", name.c_str());
    }

    ShaderSlot& ref = *slot;
    slots_.emplace(name, std::move(slot));
    return ref;
}

void ShaderWatcher::Poll()
{
    const auto now = std::chrono::steady_clock::now();
    if (now - lastPoll_ < pollInterval_) return;
    lastPoll_ = now;

    for (auto& entry : slots_) {
        ShaderSlot& slot = *entry.second;

        bool changed = false;
        fs::file_time_type time{};

        if (!slot.vsPath_.empty() && TryGetMTime(slot.vsPath_, time) && time != slot.vsTime_) {
            slot.vsTime_ = time;
            changed = true;
        }
        if (!slot.fsPath_.empty() && TryGetMTime(slot.fsPath_, time) && time != slot.fsTime_) {
            slot.fsTime_ = time;
            changed = true;
        }

        if (changed) TryReload(slot);
    }
}

bool ShaderWatcher::TryReload(ShaderSlot& slot)
{
    const bool wantsCustom = !slot.vsPath_.empty() || !slot.fsPath_.empty();
    Shader fresh = LoadShader(PathOrNull(slot.vsPath_), PathOrNull(slot.fsPath_));

    if (!LoadSucceeded(fresh, wantsCustom)) {
        // Echec : on garde l'ancien shader actif, l'erreur GLSL detaillee a
        // deja ete loguee par raylib.
        TraceLog(LOG_WARNING, "SHADERS: [%s] reload echoue, ancien shader conserve", slot.name_.c_str());
        UnloadShaderSafe(fresh);
        return false;
    }

    UnloadShaderSafe(slot.shader_);
    slot.shader_ = fresh;
    slot.valid_ = true;
    ++slot.reloadCount_;
    TraceLog(LOG_INFO, "SHADERS: [%s] recharge avec succes (#%d)", slot.name_.c_str(), slot.reloadCount_);

    if (onReload_) onReload_(slot.name_, slot.shader_);
    return true;
}

ShaderSlot* ShaderWatcher::Find(const std::string& name)
{
    auto it = slots_.find(name);
    return (it != slots_.end()) ? it->second.get() : nullptr;
}

void ShaderWatcher::SetPollInterval(double seconds)
{
    if (seconds < 0.0) seconds = 0.0;
    pollInterval_ = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(seconds));
}

void ShaderWatcher::UnloadAll()
{
    // Contexte GL deja ferme -> ne plus toucher au GPU (fin de process).
    const bool gpuReady = IsWindowReady();
    for (auto& entry : slots_) {
        if (gpuReady) UnloadShaderSafe(entry.second->shader_);
    }
    slots_.clear();
}

} // namespace racer::engine
