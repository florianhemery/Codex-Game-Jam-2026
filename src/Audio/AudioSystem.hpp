/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural placeholder audio: engine drone, biome ambiance, menu music
*/

#ifndef AUDIO_SYSTEM_HPP_
#define AUDIO_SYSTEM_HPP_

#include "raylib.h"

#include "World/Chunk/ChunkTypes.hpp"

namespace racer::audio {

class AudioSystem {
public:
    AudioSystem() = default;
    ~AudioSystem() = default;

    AudioSystem(const AudioSystem &) = delete;
    AudioSystem &operator=(const AudioSystem &) = delete;

    void init();
    void shutdown();

    void update(float dt, bool inMenu, bool inOpenWorld,
        racer::world::BiomeId biome, float speed, float maxSpeed);

private:
    void updateMenuMusic(bool active);
    void updateEngineDrone(bool active, float speed, float maxSpeed);
    void updateAmbiance(float dt, bool active, racer::world::BiomeId biome);

    Sound menuMusic_{};
    Sound engineDrone_{};
    Sound ambientCoast_{};
    Sound ambientForest_{};
    Sound ambientPort_{};
    Sound ambientVolcano_{};

    float ambientVol_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool ready_ = false;
    bool weOpenedDevice_ = false;
};

} // namespace racer::audio

#endif /* !AUDIO_SYSTEM_HPP_ */
