/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural placeholder audio: engine drone, biome ambiance, menu music
*/

#include "Audio/AudioSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace racer::audio {

namespace {

constexpr int kSampleRate = 44100;
constexpr float kPi = 3.14159265358979323846f;

enum class Waveform { Sine, Saw, Square };

float noiseAt(int index)
{
    uint32_t n = static_cast<uint32_t>(index);

    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    return 1.0f - static_cast<float>(n & 0x7fffffffU) / 1073741824.0f;
}

float softClip(float sample)
{
    return sample / (1.0f + std::fabs(sample) * 0.6f);
}

// Convertit un flux d'echantillons [-1,1] genere par sampleAt(i, t) en Wave
// PCM 16 bits mono. Point d'entree unique pour toute synthese du module.
template <typename SampleFn>
Wave buildWave(int sampleCount, SampleFn sampleAt)
{
    Wave wave{};

    wave.frameCount = static_cast<unsigned int>(sampleCount);
    wave.sampleRate = kSampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc(static_cast<unsigned int>(sampleCount) * sizeof(short));
    auto *samples = static_cast<short *>(wave.data);

    for (int i = 0; i < sampleCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSampleRate);
        const float sample = softClip(sampleAt(i, t));

        samples[i] = static_cast<short>(
            std::clamp(sample, -1.0f, 1.0f) * 30000.0f);
    }
    return wave;
}

// Oscillateur simple sur une duree donnee, utilise pour les notes de la
// musique de menu et les composantes toniques des ambiances.
Wave generateToneWave(float frequency, float durationSeconds, Waveform waveform)
{
    const int sampleCount = static_cast<int>(durationSeconds * kSampleRate);

    return buildWave(sampleCount, [=](int /*i*/, float t) {
        const float phase = std::fmod(t * frequency, 1.0f);

        switch (waveform) {
        case Waveform::Saw:
            return 2.0f * phase - 1.0f;
        case Waveform::Square:
            return phase < 0.5f ? 1.0f : -1.0f;
        case Waveform::Sine:
        default:
            return std::sin(2.0f * kPi * frequency * t);
        }
    });
}

Sound loadWave(Wave wave)
{
    Sound sound = LoadSoundFromWave(wave);

    UnloadWave(wave);
    return sound;
}

// Bourdon moteur : fondamentale en dent de scie + harmonique additive,
// boucle courte rejouee en continu avec le pitch modulé par la vitesse.
Sound makeEngineDroneWave()
{
    const float baseFreq = 85.0f;
    const int cycles = 10;
    const int sampleCount = static_cast<int>(
        static_cast<float>(cycles) * static_cast<float>(kSampleRate) / baseFreq);

    Wave wave = buildWave(sampleCount, [=](int /*i*/, float t) {
        const float fundamental = 2.0f * std::fmod(t * baseFreq, 1.0f) - 1.0f;
        const float harmonic = std::sin(2.0f * kPi * baseFreq * 2.0f * t) * 0.35f;
        const float subHarmonic = std::sin(2.0f * kPi * baseFreq * 0.5f * t) * 0.25f;

        return fundamental * 0.55f + harmonic + subHarmonic;
    });
    return loadWave(wave);
}

// Houle cotiere : bruit filtre passe-bas (moyenne glissante) module par une
// enveloppe sinusoidale lente pour simuler le va-et-vient des vagues.
Sound makeCoastAmbientWave()
{
    const float duration = 4.0f;
    const int sampleCount = static_cast<int>(duration * kSampleRate);
    float lp = 0.0f;

    Wave wave = buildWave(sampleCount, [&lp](int i, float t) mutable {
        lp += (noiseAt(i) - lp) * 0.02f;
        const float swell = 0.55f + 0.45f * std::sin(2.0f * kPi * 0.18f * t);

        return lp * 1.8f * swell;
    });
    return loadWave(wave);
}

// Foret : bruit plus haut-perche (moins filtre) avec un tremolo rapide pour
// evoquer le bruissement du feuillage.
Sound makeForestAmbientWave()
{
    const float duration = 3.5f;
    const int sampleCount = static_cast<int>(duration * kSampleRate);
    float lp = 0.0f;

    Wave wave = buildWave(sampleCount, [&lp](int i, float t) mutable {
        lp += (noiseAt(i) - lp) * 0.12f;
        const float rustle = 0.6f + 0.4f * std::sin(2.0f * kPi * 3.1f * t)
            * std::sin(2.0f * kPi * 0.7f * t);

        return lp * 1.1f * rustle;
    });
    return loadWave(wave);
}

// Port : ressac grave + bruit d'eau, ponctue de tintements metalliques
// occasionnels (cordages / bouees).
Sound makePortAmbientWave()
{
    const float duration = 4.5f;
    const int sampleCount = static_cast<int>(duration * kSampleRate);
    float lp = 0.0f;

    Wave wave = buildWave(sampleCount, [&lp](int i, float t) mutable {
        lp += (noiseAt(i) - lp) * 0.03f;
        const float water = lp * 1.4f;
        const float rumble = std::sin(2.0f * kPi * 55.0f * t) * 0.10f;
        const float clangPhase = std::fmod(t * 0.35f, 1.0f);
        const float clang = clangPhase < 0.03f
            ? std::sin(2.0f * kPi * 1400.0f * t) * std::exp(-clangPhase * 90.0f) * 0.5f
            : 0.0f;

        return water + rumble + clang;
    });
    return loadWave(wave);
}

// Volcan : rumeur infrabasse continue + craquements de bruit courts et
// espaces, simulant des coulees et projections lointaines.
Sound makeVolcanoAmbientWave()
{
    const float duration = 4.0f;
    const int sampleCount = static_cast<int>(duration * kSampleRate);

    Wave wave = buildWave(sampleCount, [](int i, float t) {
        const float rumble = std::sin(2.0f * kPi * 32.0f * t) * 0.5f
            + std::sin(2.0f * kPi * 21.0f * t) * 0.3f;
        const float cracklePhase = std::fmod(t * 0.6f, 1.0f);
        const float crackle = cracklePhase < 0.08f
            ? noiseAt(i) * std::exp(-cracklePhase * 30.0f) * 0.6f
            : 0.0f;

        return rumble + crackle;
    });
    return loadWave(wave);
}

// Musique de menu : courte arpege majeur boucle, assemblee note par note a
// partir de generateToneWave() avec une enveloppe attaque/decroissance.
Sound makeMenuArpeggioWave()
{
    const float notes[] = {261.63f, 329.63f, 392.00f, 523.25f, 392.00f, 329.63f};
    const float noteDuration = 0.30f;
    const int noteCount = static_cast<int>(sizeof(notes) / sizeof(notes[0]));
    const int samplesPerNote = static_cast<int>(noteDuration * kSampleRate);
    const int sampleCount = samplesPerNote * noteCount;

    Wave wave{};
    wave.frameCount = static_cast<unsigned int>(sampleCount);
    wave.sampleRate = kSampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc(static_cast<unsigned int>(sampleCount) * sizeof(short));

    auto *out = static_cast<short *>(wave.data);

    for (int n = 0; n < noteCount; ++n) {
        Wave noteWave = generateToneWave(notes[n], noteDuration, Waveform::Sine);
        auto *noteSamples = static_cast<short *>(noteWave.data);

        for (int i = 0; i < samplesPerNote; ++i) {
            const float noteT = static_cast<float>(i) / static_cast<float>(kSampleRate);
            const float envelope = std::sin(kPi * std::min(1.0f, noteT / noteDuration))
                * std::exp(-noteT * 2.0f);

            out[n * samplesPerNote + i] = static_cast<short>(std::clamp(
                static_cast<float>(noteSamples[i]) * envelope, -30000.0f, 30000.0f));
        }
        UnloadWave(noteWave);
    }
    return loadWave(wave);
}

} // namespace

void AudioSystem::init()
{
    weOpenedDevice_ = !IsAudioDeviceReady();
    if (weOpenedDevice_) {
        InitAudioDevice();
    }
    if (!IsAudioDeviceReady()) {
        return;
    }
    menuMusic_ = makeMenuArpeggioWave();
    engineDrone_ = makeEngineDroneWave();
    ambientCoast_ = makeCoastAmbientWave();
    ambientForest_ = makeForestAmbientWave();
    ambientPort_ = makePortAmbientWave();
    ambientVolcano_ = makeVolcanoAmbientWave();
    ready_ = true;
}

void AudioSystem::shutdown()
{
    if (!ready_) {
        return;
    }
    StopSound(menuMusic_);
    StopSound(engineDrone_);
    StopSound(ambientCoast_);
    StopSound(ambientForest_);
    StopSound(ambientPort_);
    StopSound(ambientVolcano_);
    UnloadSound(menuMusic_);
    UnloadSound(engineDrone_);
    UnloadSound(ambientCoast_);
    UnloadSound(ambientForest_);
    UnloadSound(ambientPort_);
    UnloadSound(ambientVolcano_);
    ready_ = false;
    if (weOpenedDevice_ && IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
}

void AudioSystem::updateMenuMusic(bool active)
{
    if (!active) {
        if (IsSoundPlaying(menuMusic_)) {
            StopSound(menuMusic_);
        }
        return;
    }
    SetSoundVolume(menuMusic_, 0.32f);
    if (!IsSoundPlaying(menuMusic_)) {
        PlaySound(menuMusic_);
    }
}

void AudioSystem::updateEngineDrone(bool active, float speed, float maxSpeed)
{
    if (!active) {
        if (IsSoundPlaying(engineDrone_)) {
            StopSound(engineDrone_);
        }
        return;
    }
    const float norm = std::clamp(
        std::fabs(speed) / std::max(maxSpeed, 1.0f), 0.0f, 1.0f);
    const float pitch = 0.7f + norm * (2.2f - 0.7f);

    SetSoundPitch(engineDrone_, pitch);
    SetSoundVolume(engineDrone_, 0.16f);
    if (!IsSoundPlaying(engineDrone_)) {
        PlaySound(engineDrone_);
    }
}

void AudioSystem::updateAmbiance(float dt, bool active, racer::world::BiomeId biome)
{
    Sound *sounds[4] = {
        &ambientCoast_, &ambientForest_, &ambientPort_, &ambientVolcano_
    };
    const int target = active ? static_cast<int>(biome) : -1;
    const float blend = std::min(1.0f, dt * 1.2f);

    for (int i = 0; i < 4; ++i) {
        const float wanted = (i == target) ? 0.42f : 0.0f;

        ambientVol_[i] += (wanted - ambientVol_[i]) * blend;
        if (ambientVol_[i] < 0.01f) {
            if (IsSoundPlaying(*sounds[i])) {
                StopSound(*sounds[i]);
            }
            continue;
        }
        SetSoundVolume(*sounds[i], ambientVol_[i]);
        if (!IsSoundPlaying(*sounds[i])) {
            PlaySound(*sounds[i]);
        }
    }
}

void AudioSystem::update(float dt, bool inMenu, bool inOpenWorld,
    racer::world::BiomeId biome, float speed, float maxSpeed)
{
    if (!ready_) {
        return;
    }
    updateMenuMusic(inMenu);
    updateEngineDrone(inOpenWorld, speed, maxSpeed);
    updateAmbiance(dt, inOpenWorld, biome);
}

} // namespace racer::audio
