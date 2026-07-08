/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural race audio feedback
*/

#ifndef AUDIO_FEEDBACK_HPP_
#define AUDIO_FEEDBACK_HPP_

#include "Race/RaceState.hpp"
#include "Vehicle/CarInput.hpp"
#include "World/Chunk/ChunkTypes.hpp"

namespace racer {
namespace app {

struct AudioDriveSnapshot {
    CarInput input{};
    float speed = 0.0f;
    float heading = 0.0f;
    float velocityHeading = 0.0f;
    float maxSpeed = 1.0f;
    float surfaceDrag = 1.0f;
    bool nitroActive = false;
    bool drifting = false;
};

class AudioFeedback {
public:
    AudioFeedback();
    ~AudioFeedback();

    AudioFeedback(const AudioFeedback &) = delete;
    AudioFeedback &operator=(const AudioFeedback &) = delete;

    void update(const RaceState &race, const AudioDriveSnapshot &drive,
        float dt, bool paused);
    void updateOpenWorld(const AudioDriveSnapshot &drive, float dt,
        racer::world::BiomeId biome, bool paused);

private:
    Sound countdownBeep_{};
    Sound goSignal_{};
    Sound victoryFanfare_{};
    Sound engineExhaust_{};
    Sound engineRasp_{};
    Sound intakeWhoosh_{};
    Sound driftScreech_{};
    Sound offroadRumble_{};
    Sound nitroWhistle_{};
    Sound brakeScreech_{};
    Sound impactThump_{};
    bool ready_ = false;

    int lastCountdownDigit_ = -1;
    RacePhase lastPhase_ = RacePhase::COUNTDOWN;
    bool victoryPlayed_ = false;
    float lastSpeed_ = 0.0f;
    bool wasHardBraking_ = false;

    float engineRpm_ = 900.0f;
    float targetRpm_ = 900.0f;
    float exhaustVol_ = 0.0f;
    float raspVol_ = 0.0f;
    float intakeVol_ = 0.0f;
    float driftVol_ = 0.0f;
    float offroadVol_ = 0.0f;
    float nitroVol_ = 0.0f;

    static constexpr float kRefEngineRpm = 3000.0f;

    void stopDrivingLoops();
    void updateEngineRpm(const AudioDriveSnapshot &drive, float dt);
    void updateRaceEvents(const RaceState &race, const AudioDriveSnapshot &drive);
    void updateDrivingMix(const AudioDriveSnapshot &drive, float dt);
    void setLoop(Sound &sound, float volume, float pitch);
};

} // namespace app
} // namespace racer

#endif /* !AUDIO_FEEDBACK_HPP_ */
