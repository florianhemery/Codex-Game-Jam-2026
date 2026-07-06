#include "common/noise/perlin.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace common::noise {

namespace {

float Fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float Grad2D(int hash, float x, float y) {
    switch (hash & 0x7) {
        case 0: return x + y;
        case 1: return -x + y;
        case 2: return x - y;
        case 3: return -x - y;
        case 4: return x;
        case 5: return -x;
        case 6: return y;
        default: return -y;
    }
}

float Grad3D(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

} // namespace

PerlinNoise::PerlinNoise(uint32_t seed) {
    std::array<int, 256> p{};
    std::iota(p.begin(), p.end(), 0);

    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.end(), rng);

    for (int i = 0; i < 256; ++i) {
        permutation_[i] = p[i];
        permutation_[i + 256] = p[i];
    }
}

float PerlinNoise::Noise2D(float x, float y) const {
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    float u = Fade(xf);
    float v = Fade(yf);

    int aa = permutation_[permutation_[xi] + yi];
    int ab = permutation_[permutation_[xi] + yi + 1];
    int ba = permutation_[permutation_[xi + 1] + yi];
    int bb = permutation_[permutation_[xi + 1] + yi + 1];

    float x1 = Lerp(u, Grad2D(aa, xf, yf), Grad2D(ba, xf - 1.0f, yf));
    float x2 = Lerp(u, Grad2D(ab, xf, yf - 1.0f), Grad2D(bb, xf - 1.0f, yf - 1.0f));
    return Lerp(v, x1, x2);
}

float PerlinNoise::Noise3D(float x, float y, float z) const {
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    int zi = static_cast<int>(std::floor(z)) & 255;
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    float zf = z - std::floor(z);

    float u = Fade(xf);
    float v = Fade(yf);
    float w = Fade(zf);

    int a = permutation_[xi] + yi;
    int aa = permutation_[a] + zi;
    int ab = permutation_[a + 1] + zi;
    int b = permutation_[xi + 1] + yi;
    int ba = permutation_[b] + zi;
    int bb = permutation_[b + 1] + zi;

    float x1 = Lerp(u, Grad3D(permutation_[aa], xf, yf, zf),
                        Grad3D(permutation_[ba], xf - 1.0f, yf, zf));
    float x2 = Lerp(u, Grad3D(permutation_[ab], xf, yf - 1.0f, zf),
                        Grad3D(permutation_[bb], xf - 1.0f, yf - 1.0f, zf));
    float y1 = Lerp(v, x1, x2);

    float x3 = Lerp(u, Grad3D(permutation_[aa + 1], xf, yf, zf - 1.0f),
                        Grad3D(permutation_[ba + 1], xf - 1.0f, yf, zf - 1.0f));
    float x4 = Lerp(u, Grad3D(permutation_[ab + 1], xf, yf - 1.0f, zf - 1.0f),
                        Grad3D(permutation_[bb + 1], xf - 1.0f, yf - 1.0f, zf - 1.0f));
    float y2 = Lerp(v, x3, x4);

    return Lerp(w, y1, y2);
}

float PerlinNoise::Fbm2D(float x, float y, int octaves, float lacunarity, float gain) const {
    float sum = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        sum += Noise2D(x * frequency, y * frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }

    return maxAmplitude > 0.0f ? sum / maxAmplitude : 0.0f;
}

float PerlinNoise::Fbm3D(float x, float y, float z, int octaves, float lacunarity, float gain) const {
    float sum = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        sum += Noise3D(x * frequency, y * frequency, z * frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }

    return maxAmplitude > 0.0f ? sum / maxAmplitude : 0.0f;
}

} // namespace common::noise
