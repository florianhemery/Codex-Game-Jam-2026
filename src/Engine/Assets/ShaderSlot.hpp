/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Hot-reloadable shader slot
*/

#ifndef SHADER_SLOT_HPP_
#define SHADER_SLOT_HPP_

#include "raylib.h"

#include <chrono>
#include <filesystem>
#include <string>

namespace racer::engine {

class ShaderWatcher;

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

} // namespace racer::engine

#endif /* !SHADER_SLOT_HPP_ */
