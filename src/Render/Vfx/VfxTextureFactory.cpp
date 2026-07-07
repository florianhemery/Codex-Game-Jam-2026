/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX procedural texture factory
*/

#include "Render/Vfx/VfxTextureFactory.hpp"

#include <algorithm>
#include <cmath>

namespace racer {

constexpr Color VfxTextureFactory::rgba(int r, int g, int b, int a)
{
    return Color{
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        static_cast<unsigned char>(a)};
}

float VfxTextureFactory::frand(float lo, float hi)
{
    float t = static_cast<float>(GetRandomValue(0, 16383)) / 16383.0f;

    return lo + (hi - lo) * t;
}

unsigned char VfxTextureFactory::lerpChannel(
    unsigned char ca, unsigned char cb, float t)
{
    float v = static_cast<float>(ca)
        + (static_cast<float>(cb) - static_cast<float>(ca)) * t;

    return static_cast<unsigned char>(std::clamp(v, 0.0f, 255.0f));
}

Color VfxTextureFactory::lerpColor(Color a, Color b, float t)
{
    return Color{
        lerpChannel(a.r, b.r, t),
        lerpChannel(a.g, b.g, t),
        lerpChannel(a.b, b.b, t),
        lerpChannel(a.a, b.a, t)};
}

void VfxTextureFactory::finishTexture(Texture2D &tex)
{
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);
}

Texture2D VfxTextureFactory::makePuffTexture()
{
    Image img = GenImageGradientRadial(
        64, 64, 0.25f,
        rgba(255, 255, 255, 255),
        rgba(255, 255, 255, 0));
    Texture2D tex = LoadTextureFromImage(img);

    UnloadImage(img);
    finishTexture(tex);
    return tex;
}

Color VfxTextureFactory::streakPixelColor(int x, int y)
{
    float u = (static_cast<float>(x) + 0.5f) / 16.0f * 2.0f - 1.0f;
    float v = (static_cast<float>(y) + 0.5f) / 64.0f;
    float bell = std::max(0.0f, 1.0f - std::fabs(u));

    bell *= bell;
    float ends = std::clamp(std::min(v, 1.0f - v) * 8.0f, 0.0f, 1.0f);

    return rgba(255, 255, 255, static_cast<int>(bell * ends * 255.0f));
}

Texture2D VfxTextureFactory::makeStreakTexture()
{
    Image img = GenImageColor(16, 64, BLANK);

    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 16; ++x)
            ImageDrawPixel(&img, x, y, streakPixelColor(x, y));
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    finishTexture(tex);
    return tex;
}

Texture2D VfxTextureFactory::makeSquareTexture()
{
    Image img = GenImageColor(8, 8, rgba(255, 255, 255, 255));
    Texture2D tex = LoadTextureFromImage(img);

    UnloadImage(img);
    finishTexture(tex);
    return tex;
}

} // namespace racer
