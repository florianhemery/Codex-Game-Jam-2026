/*
** EPITECH PROJECT, 2026
** racer
** File description:
** shader uniform locations
*/

#include "Engine/Render/ShaderLocations.hpp"

#include "rlgl.h"

namespace racer::engine {

int locOrArray(const Shader &shader, const char *name, const char *arrayName)
{
    int loc = GetShaderLocation(shader, name);

    if (loc == -1)
        loc = GetShaderLocation(shader, arrayName);
    return loc;
}

void refreshLitLocs(const Shader &lit, LitLocs &locs)
{
    locs.viewPos = GetShaderLocation(lit, "viewPos");
    locs.sunDir = GetShaderLocation(lit, "sunDir");
    locs.sunColor = GetShaderLocation(lit, "sunColor");
    locs.skyAmbient = GetShaderLocation(lit, "skyAmbient");
    locs.groundAmbient = GetShaderLocation(lit, "groundAmbient");
    locs.fogColor = GetShaderLocation(lit, "fogColor");
    locs.fogDensity = GetShaderLocation(lit, "fogDensity");
    locs.lightVP = GetShaderLocation(lit, "lightVP");
    locs.shadowMap = GetShaderLocation(lit, "shadowMap");
    locs.shadowTexel = GetShaderLocation(lit, "shadowTexel");
    locs.lightsPos = locOrArray(lit, "lightsPos", "lightsPos[0]");
    locs.lightsColor = locOrArray(lit, "lightsColor", "lightsColor[0]");
    locs.lightsCount = GetShaderLocation(lit, "lightsCount");
}

void refreshSkyLocs(const Shader &sky, SkyLocs &locs)
{
    locs.sunDir = GetShaderLocation(sky, "sunDir");
    locs.sunColor = GetShaderLocation(sky, "sunColor");
    locs.zenith = GetShaderLocation(sky, "skyZenith");
    locs.horizon = GetShaderLocation(sky, "skyHorizon");
    locs.coverage = GetShaderLocation(sky, "cloudCoverage");
    locs.tint = GetShaderLocation(sky, "cloudTint");
    locs.fogColor = GetShaderLocation(sky, "fogColor");
    locs.time = GetShaderLocation(sky, "time");
    locs.stars = GetShaderLocation(sky, "starsOn");
}

void refreshPostLocs(const Shader &post, PostLocs &locs)
{
    locs.exposure = GetShaderLocation(post, "exposure");
    locs.gradeTint = GetShaderLocation(post, "gradeTint");
    locs.saturation = GetShaderLocation(post, "saturation");
    locs.vignette = GetShaderLocation(post, "vignette");
    locs.aberration = GetShaderLocation(post, "aberration");
    locs.grainAmount = GetShaderLocation(post, "grainAmount");
    locs.time = GetShaderLocation(post, "time");
    locs.speedBlur = GetShaderLocation(post, "speedBlur");
}

void bindShadowMapTexture(const Shader &lit, const LitLocs &locs,
                            unsigned int textureId, int shadowSlot)
{
    if (locs.shadowMap == -1)
        return;

    rlEnableShader(lit.id);
    int slot = shadowSlot;

    rlActiveTextureSlot(shadowSlot);
    rlEnableTexture(textureId);
    rlSetUniform(locs.shadowMap, &slot, SHADER_UNIFORM_INT, 1);
    rlActiveTextureSlot(0);
}

} // namespace racer::engine
