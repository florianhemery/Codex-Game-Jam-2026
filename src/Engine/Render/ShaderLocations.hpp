/*
** EPITECH PROJECT, 2026
** racer
** File description:
** shader uniform locations
*/

#ifndef SHADER_LOCATIONS_HPP_
#define SHADER_LOCATIONS_HPP_

#include "raylib.h"

namespace racer::engine {

enum class Ambiance { MIDI, AUBE_DOREE, CREPUSCULE, ORAGE };

struct AmbianceParams {
    Vector3 sunDir;
    Vector3 sunColor;
    Vector3 skyAmbient;
    Vector3 groundAmbient;
    Vector3 skyZenith;
    Vector3 skyHorizon;
    float cloudCoverage;
    Vector3 cloudTint;
    Vector3 fogColor;
    float fogDensity;
    float exposure;
    Vector3 gradeTint;
    float saturation;
    float vignette;
    bool headlights;
    bool stars;
};

struct LitLocs {
    int viewPos = -1;
    int sunDir = -1;
    int sunColor = -1;
    int skyAmbient = -1;
    int groundAmbient = -1;
    int fogColor = -1;
    int fogDensity = -1;
    int lightVP = -1;
    int shadowMap = -1;
    int shadowTexel = -1;
    int lightsPos = -1;
    int lightsColor = -1;
    int lightsCount = -1;
    int useTextureAlbedo = -1;
    int terrainMode = -1;
    int biomeTint = -1;
};

struct SkyLocs {
    int sunDir = -1;
    int sunColor = -1;
    int zenith = -1;
    int horizon = -1;
    int coverage = -1;
    int tint = -1;
    int fogColor = -1;
    int time = -1;
    int stars = -1;
};

struct PostLocs {
    int exposure = -1;
    int gradeTint = -1;
    int saturation = -1;
    int vignette = -1;
    int aberration = -1;
    int grainAmount = -1;
    int time = -1;
    int speedBlur = -1;
};

int locOrArray(const Shader &shader, const char *name, const char *arrayName);

void refreshLitLocs(const Shader &lit, LitLocs &locs);
void refreshSkyLocs(const Shader &sky, SkyLocs &locs);
void refreshPostLocs(const Shader &post, PostLocs &locs);

void bindShadowMapTexture(const Shader &lit, const LitLocs &locs,
                            unsigned int textureId, int shadowSlot);

} // namespace racer::engine

#endif /* !SHADER_LOCATIONS_HPP_ */
