#pragma once

#include <array>
#include <cstdint>

namespace common::noise {

// Bruit de Perlin ameliore (Ken Perlin, 2002), seedable. Fbm2D/Fbm3D sommes
// plusieurs octaves pour un relief naturel (terrain, biomes, grottes).
class PerlinNoise {
public:
    explicit PerlinNoise(uint32_t seed);

    float Noise2D(float x, float y) const;
    float Noise3D(float x, float y, float z) const;

    float Fbm2D(float x, float y, int octaves, float lacunarity, float gain) const;
    float Fbm3D(float x, float y, float z, int octaves, float lacunarity, float gain) const;

private:
    std::array<int, 512> permutation_{};
};

} // namespace common::noise
