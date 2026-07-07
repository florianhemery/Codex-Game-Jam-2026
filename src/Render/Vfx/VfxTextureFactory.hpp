/*
** EPITECH PROJECT, 2026
** racer
** File description:
** VFX procedural texture factory
*/

#ifndef VFX_TEXTURE_FACTORY_HPP_
#define VFX_TEXTURE_FACTORY_HPP_

#include "raylib.h"

namespace racer {

struct VfxTextureFactory {
    static constexpr Color rgba(int r, int g, int b, int a);
    static float frand(float lo, float hi);
    static unsigned char lerpChannel(
        unsigned char ca, unsigned char cb, float t);
    static Color lerpColor(Color a, Color b, float t);
    static void finishTexture(Texture2D &tex);
    static Texture2D makePuffTexture();
    static Color streakPixelColor(int x, int y);
    static Texture2D makeStreakTexture();
    static Texture2D makeSquareTexture();
};

} // namespace racer

#endif /* !VFX_TEXTURE_FACTORY_HPP_ */
