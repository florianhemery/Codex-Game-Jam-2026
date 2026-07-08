/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Lit-shader car draw state (colDiffuse + no texture sampling)
*/

#include "Render/Car/CarDraw.hpp"

#include "rlgl.h"

namespace racer {
namespace carDraw {

namespace {

Shader g_lit{};
int g_colLoc = -1;
int g_noTexLoc = -1;
bool g_active = false;

void uploadDiffuse(Color color)
{
    if (!g_active || g_colLoc < 0)
        return;
    const float diffuse[4] = {
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f,
        static_cast<float>(color.a) / 255.0f,
    };

    SetShaderValue(g_lit, g_colLoc, diffuse, SHADER_UNIFORM_VEC4);
}

} // namespace

void beginPass(Shader lit, Color fallbackTint)
{
    g_lit = lit;
    g_active = (lit.id != 0);

    rlDrawRenderBatchActive();
    EndBlendMode();
    rlSetTexture(rlGetTextureIdDefault());
    rlEnableDepthMask();
    rlEnableDepthTest();
    rlEnableBackfaceCulling();

    if (!g_active)
        return;

    g_colLoc = GetShaderLocation(lit, "colDiffuse");
    g_noTexLoc = GetShaderLocation(lit, "useTextureAlbedo");

    if (g_noTexLoc >= 0) {
        const float noTex = 0.0f;

        SetShaderValue(lit, g_noTexLoc, &noTex, SHADER_UNIFORM_FLOAT);
    }
    uploadDiffuse(fallbackTint);
}

void endPass(Shader lit)
{
    rlDrawRenderBatchActive();
    rlSetTexture(rlGetTextureIdDefault());
    EndBlendMode();

    if (!g_active || lit.id == 0) {
        g_active = false;
        g_lit = {};
        return;
    }

    if (g_noTexLoc >= 0) {
        const float useTex = 1.0f;

        SetShaderValue(lit, g_noTexLoc, &useTex, SHADER_UNIFORM_FLOAT);
    }
    if (g_colLoc >= 0) {
        const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        SetShaderValue(lit, g_colLoc, white, SHADER_UNIFORM_VEC4);
    }
    g_active = false;
    g_lit = {};
}

void setColor(Color color)
{
    uploadDiffuse(color);
}

bool isActive()
{
    return g_active;
}

} // namespace carDraw
} // namespace racer
