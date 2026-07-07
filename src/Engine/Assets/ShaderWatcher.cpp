/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Shader hot-reload implementation
*/

#include "Engine/Assets/ShaderWatcher.hpp"

#include "rlgl.h"

#include <utility>

namespace racer::engine {

namespace fs = std::filesystem;

class ShaderLoadUtils {
public:
    static bool tryGetMTime(
        const std::string &path, fs::file_time_type &out);
    static const char *pathOrNull(const std::string &path);
    static bool loadSucceeded(const Shader &shader, bool wantsCustomStages);
    static void unloadShaderSafe(Shader &shader);
    static bool checkPathChanged(const std::string &path,
                                 fs::file_time_type &storedTime);
};

bool ShaderLoadUtils::tryGetMTime(
    const std::string &path, fs::file_time_type &out)
{
    std::error_code ec;
    const fs::file_time_type time = fs::last_write_time(path, ec);

    if (ec)
        return false;
    out = time;
    return true;
}

const char *ShaderLoadUtils::pathOrNull(const std::string &path)
{
    return path.empty() ? nullptr : path.c_str();
}

bool ShaderLoadUtils::loadSucceeded(
    const Shader &shader, bool wantsCustomStages)
{
    if (!IsShaderValid(shader))
        return false;
    if (wantsCustomStages && shader.id == rlGetShaderIdDefault())
        return false;
    return true;
}

void ShaderLoadUtils::unloadShaderSafe(Shader &shader)
{
    if (shader.id == 0) {
        shader = Shader{};
        return;
    }
    if (shader.id == rlGetShaderIdDefault()) {
        if (shader.locs != nullptr)
            MemFree(shader.locs);
    } else {
        UnloadShader(shader);
    }
    shader = Shader{};
}

bool ShaderLoadUtils::checkPathChanged(const std::string &path,
                                       fs::file_time_type &storedTime)
{
    fs::file_time_type time{};

    if (path.empty() || !tryGetMTime(path, time) || time == storedTime)
        return false;
    storedTime = time;
    return true;
}

ShaderWatcher::~ShaderWatcher()
{
    UnloadAll();
}

void ShaderWatcher::initSlotMtimes(ShaderSlot &slot,
                                   const std::string &name,
                                   const std::string &vsPath,
                                   const std::string &fsPath)
{
    if (!vsPath.empty()
        && !ShaderLoadUtils::tryGetMTime(vsPath, slot.vsTime_)) {
        slot.vsTime_ = fs::file_time_type::min();
        TraceLog(LOG_WARNING,
            "SHADERS: [%s] vertex introuvable: %s",
            name.c_str(), vsPath.c_str());
    }
    if (!fsPath.empty()
        && !ShaderLoadUtils::tryGetMTime(fsPath, slot.fsTime_)) {
        slot.fsTime_ = fs::file_time_type::min();
        TraceLog(LOG_WARNING,
            "SHADERS: [%s] fragment introuvable: %s",
            name.c_str(), fsPath.c_str());
    }
}

void ShaderWatcher::loadSlotInitial(ShaderSlot &slot,
                                    const std::string &vsPath,
                                    const std::string &fsPath)
{
    const bool wantsCustom = !vsPath.empty() || !fsPath.empty();

    slot.shader_ = LoadShader(
        ShaderLoadUtils::pathOrNull(vsPath),
        ShaderLoadUtils::pathOrNull(fsPath));
    slot.valid_ = ShaderLoadUtils::loadSucceeded(slot.shader_, wantsCustom);
    if (!slot.valid_) {
        TraceLog(LOG_WARNING,
            "SHADERS: [%s] compilation initiale echouee, "
            "shader par defaut actif",
            slot.name_.c_str());
    }
}

ShaderSlot &ShaderWatcher::RegisterShader(const std::string &name,
                                          const std::string &vsPath,
                                          const std::string &fsPath)
{
    if (auto it = slots_.find(name); it != slots_.end()) {
        TraceLog(LOG_WARNING,
            "SHADERS: [%s] deja enregistre, slot existant renvoye",
            name.c_str());
        return *it->second;
    }

    auto slot = std::make_unique<ShaderSlot>();
    slot->name_ = name;
    slot->vsPath_ = vsPath;
    slot->fsPath_ = fsPath;
    initSlotMtimes(*slot, name, vsPath, fsPath);
    loadSlotInitial(*slot, vsPath, fsPath);

    ShaderSlot &ref = *slot;
    slots_.emplace(name, std::move(slot));
    return ref;
}

void ShaderWatcher::Poll()
{
    const auto now = std::chrono::steady_clock::now();

    if (now - lastPoll_ < pollInterval_)
        return;
    lastPoll_ = now;

    for (auto &entry : slots_) {
        ShaderSlot &slot = *entry.second;
        bool changed = false;

        if (ShaderLoadUtils::checkPathChanged(slot.vsPath_, slot.vsTime_))
            changed = true;
        if (ShaderLoadUtils::checkPathChanged(slot.fsPath_, slot.fsTime_))
            changed = true;
        if (changed)
            TryReload(slot);
    }
}

bool ShaderWatcher::TryReload(ShaderSlot &slot)
{
    const bool wantsCustom = !slot.vsPath_.empty() || !slot.fsPath_.empty();
    Shader fresh = LoadShader(
        ShaderLoadUtils::pathOrNull(slot.vsPath_),
        ShaderLoadUtils::pathOrNull(slot.fsPath_));

    if (!ShaderLoadUtils::loadSucceeded(fresh, wantsCustom)) {
        TraceLog(LOG_WARNING,
            "SHADERS: [%s] reload echoue, ancien shader conserve",
            slot.name_.c_str());
        ShaderLoadUtils::unloadShaderSafe(fresh);
        return false;
    }

    ShaderLoadUtils::unloadShaderSafe(slot.shader_);
    slot.shader_ = fresh;
    slot.valid_ = true;
    ++slot.reloadCount_;
    TraceLog(LOG_INFO,
        "SHADERS: [%s] recharge avec succes (#%d)",
        slot.name_.c_str(), slot.reloadCount_);

    if (onReload_)
        onReload_(slot.name_, slot.shader_);
    return true;
}

ShaderSlot *ShaderWatcher::Find(const std::string &name)
{
    auto it = slots_.find(name);

    if (it != slots_.end())
        return it->second.get();
    return nullptr;
}

void ShaderWatcher::SetPollInterval(double seconds)
{
    if (seconds < 0.0)
        seconds = 0.0;
    pollInterval_ = std::chrono::duration_cast<
        std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(seconds));
}

void ShaderWatcher::UnloadAll()
{
    const bool gpuReady = IsWindowReady();

    for (auto &entry : slots_) {
        if (gpuReady)
            ShaderLoadUtils::unloadShaderSafe(entry.second->shader_);
    }
    slots_.clear();
}

} // namespace racer::engine
