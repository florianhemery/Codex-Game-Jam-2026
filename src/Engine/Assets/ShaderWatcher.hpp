/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Hot-reload shader registry and file watcher
*/

#ifndef SHADER_WATCHER_HPP_
#define SHADER_WATCHER_HPP_

#include "raylib.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace racer::engine {

class ShaderSlot {
public:
    Shader &get()
    {
        return shader_;
    }

    const Shader &get() const
    {
        return shader_;
    }

    const std::string &name() const
    {
        return name_;
    }

    const std::string &vsPath() const
    {
        return vsPath_;
    }

    const std::string &fsPath() const
    {
        return fsPath_;
    }

    bool isValid() const
    {
        return valid_;
    }

    int reloadCount() const
    {
        return reloadCount_;
    }

private:
    friend class ShaderWatcher;

    Shader shader_{};
    std::string name_;
    std::string vsPath_;
    std::string fsPath_;
    std::filesystem::file_time_type vsTime_{};
    std::filesystem::file_time_type fsTime_{};
    bool valid_ = false;
    int reloadCount_ = 0;
};

class ShaderWatcher {
public:
    using ReloadCallback = std::function<void(
        const std::string &name, Shader &shader)>;

    ShaderWatcher() = default;
    ~ShaderWatcher();

    ShaderWatcher(const ShaderWatcher &) = delete;
    ShaderWatcher &operator=(const ShaderWatcher &) = delete;

    ShaderSlot &registerShader(const std::string &name,
                               const std::string &vsPath,
                               const std::string &fsPath);

    void poll();

    ShaderSlot *find(const std::string &name);

    void setOnReload(ReloadCallback callback)
    {
        onReload_ = std::move(callback);
    }

    void setPollInterval(double seconds);

    void unloadAll();

    int shaderCount() const
    {
        return static_cast<int>(slots_.size());
    }

private:
    bool tryReload(ShaderSlot &slot);

    void initSlotMtimes(ShaderSlot &slot,
                        const std::string &name,
                        const std::string &vsPath,
                        const std::string &fsPath);

    void loadSlotInitial(ShaderSlot &slot,
                         const std::string &vsPath,
                         const std::string &fsPath);

    std::unordered_map<std::string, std::unique_ptr<ShaderSlot>> slots_;
    ReloadCallback onReload_;
    std::chrono::steady_clock::duration pollInterval_ =
        std::chrono::milliseconds(250);
    std::chrono::steady_clock::time_point lastPoll_{};
};

} // namespace racer::engine

#endif /* !SHADER_WATCHER_HPP_ */
