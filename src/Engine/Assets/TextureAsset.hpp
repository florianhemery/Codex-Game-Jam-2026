/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Loaded 2D texture asset
*/

#ifndef TEXTURE_ASSET_HPP_
#define TEXTURE_ASSET_HPP_

#include "raylib.h"

#include <string>

namespace racer::engine {

class AssetRegistry;
class AssetRegistryDetail;

class TextureAsset {
public:
    Texture2D &get()
    {
        return texture_;
    }

    const Texture2D &get() const
    {
        return texture_;
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

    Texture2D texture_{};
    std::string path_;
    bool placeholder_ = false;
    int refCount_ = 0;
};

} // namespace racer::engine

#endif /* !TEXTURE_ASSET_HPP_ */
