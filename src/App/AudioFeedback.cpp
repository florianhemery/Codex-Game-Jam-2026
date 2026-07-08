/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural race audio feedback — arcade V8 thermal engine synth
*/

#include "App/AudioFeedback.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace racer {
namespace app {

namespace {

constexpr int kSampleRate = 44100;
constexpr float kPi = 3.14159265358979323846f;

float normalizeAngle(float angle)
{
    while (angle > kPi) {
        angle -= 2.0f * kPi;
    }
    while (angle < -kPi) {
        angle += 2.0f * kPi;
    }
    return angle;
}

float whiteNoise(int index)
{
    uint32_t n = static_cast<uint32_t>(index);

    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    return 1.0f - static_cast<float>(n & 0x7fffffffU) / 1073741824.0f;
}

float softClip(float sample)
{
    return sample / (1.0f + std::fabs(sample) * 0.65f);
}

Wave makeWaveBuffer(int sampleCount, auto sampleAt)
{
    Wave wave{};

    wave.frameCount = sampleCount;
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

Sound loadWave(Wave wave)
{
    Sound sound = LoadSoundFromWave(wave);

    UnloadWave(wave);
    return sound;
}

// Boucle moteur V8 : impulsions de combustion + harmoniques + rasp échappement.
Sound makeEngineExhaustLoop()
{
    const float refRpm = 3000.0f;
    const float crankHz = refRpm / 60.0f;
    const float firingHz = crankHz * 4.0f;
    const int cycles = 6;
    const int sampleCount = static_cast<int>(
        static_cast<float>(cycles) * static_cast<float>(kSampleRate) / crankHz);

    Wave wave = makeWaveBuffer(sampleCount, [=](int i, float t) {
        const float crankPhase = std::fmod(t * crankHz, 1.0f);
        const float firePhase = std::fmod(t * firingHz, 1.0f);

        float pulse = 0.0f;
        for (int k = 0; k < 4; ++k) {
            const float p = std::fmod(firePhase + static_cast<float>(k) * 0.25f, 1.0f);
            const float env = std::exp(-p * 11.0f);
            const float click = std::exp(-p * 38.0f);

            pulse += env * (0.72f + click * 0.28f);
        }
        pulse *= 0.22f;

        const float rumble = std::sin(2.0f * kPi * crankHz * t) * 0.34f
            + std::sin(2.0f * kPi * crankHz * 2.0f * t) * 0.18f
            + std::sin(2.0f * kPi * crankHz * 0.5f * t) * 0.12f;

        const float saw = (2.0f * crankPhase - 1.0f) * 0.10f;
        const float oddHarm = std::sin(2.0f * kPi * firingHz * t) * 0.14f
            + std::sin(2.0f * kPi * firingHz * 3.0f * t) * 0.06f;

        const float fireNoise = whiteNoise(i) * std::exp(-firePhase * 7.0f) * 0.16f;
        const float am = 0.78f + 0.22f * std::sin(2.0f * kPi * crankHz * t);

        return (pulse + rumble + saw + oddHarm + fireNoise) * am;
    });
    return loadWave(wave);
}

// Couche aiguë : harmoniques hautes + grain d'échappement.
Sound makeEngineRaspLoop()
{
    const float refRpm = 3000.0f;
    const float crankHz = refRpm / 60.0f;
    const float firingHz = crankHz * 4.0f;
    const int cycles = 5;
    const int sampleCount = static_cast<int>(
        static_cast<float>(cycles) * static_cast<float>(kSampleRate) / crankHz);

    Wave wave = makeWaveBuffer(sampleCount, [=](int i, float t) {
        const float firePhase = std::fmod(t * firingHz, 1.0f);
        const float env = std::exp(-firePhase * 5.5f);
        const float hi = std::sin(2.0f * kPi * firingHz * 2.0f * t) * 0.22f
            + std::sin(2.0f * kPi * firingHz * 3.5f * t) * 0.14f;
        const float noise = whiteNoise(i + 17) * env * 0.42f;
        const float ring = std::sin(2.0f * kPi * (firingHz * 1.7f) * t) * env * 0.18f;

        return (hi + noise + ring) * (0.55f + 0.45f * env);
    });
    return loadWave(wave);
}

// Aspiration / papillon à l'accélération.
Sound makeIntakeLoop()
{
    const float duration = 1.6f;
    const int sampleCount = static_cast<int>(duration * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int i, float t) {
        const float wobble = 0.65f + 0.35f * std::sin(2.0f * kPi * 2.4f * t);
        const float tone = std::sin(2.0f * kPi * 180.0f * t) * 0.12f
            + std::sin(2.0f * kPi * 360.0f * t) * 0.08f;
        const float noise = whiteNoise(i + 91) * 0.55f;
        const float filtered = noise * (0.45f + 0.55f * std::sin(2.0f * kPi * 0.8f * t));

        return (tone + filtered) * wobble * 0.75f;
    });
    return loadWave(wave);
}

Sound makeDriftScreechLoop()
{
    const float duration = 2.2f;
    const int sampleCount = static_cast<int>(duration * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int i, float t) {
        const float n = whiteNoise(i + 3);
        const float band1 = n * std::sin(2.0f * kPi * 940.0f * t);
        const float band2 = whiteNoise(i + 33) * std::sin(2.0f * kPi * 1580.0f * t);
        const float band3 = whiteNoise(i + 63) * std::sin(2.0f * kPi * 2360.0f * t);
        const float am = 0.50f + 0.50f * std::sin(2.0f * kPi * 13.0f * t);

        return (band1 * 0.40f + band2 * 0.32f + band3 * 0.22f) * am;
    });
    return loadWave(wave);
}

Sound makeOffroadLoop()
{
    const float duration = 2.0f;
    const int sampleCount = static_cast<int>(duration * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int i, float t) {
        const float low = std::sin(2.0f * kPi * 42.0f * t) * 0.40f
            + std::sin(2.0f * kPi * 67.0f * t) * 0.22f;
        const float gravel = whiteNoise(i + 7) * 0.50f;
        const float am = 0.60f + 0.40f * std::sin(2.0f * kPi * 5.5f * t);

        return (low + gravel * 0.65f) * am;
    });
    return loadWave(wave);
}

Sound makeNitroWhistleLoop()
{
    const float duration = 1.4f;
    const int sampleCount = static_cast<int>(duration * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int i, float t) {
        const float whistle = std::sin(2.0f * kPi * 920.0f * t) * 0.22f
            + std::sin(2.0f * kPi * 1380.0f * t) * 0.16f;
        const float noise = whiteNoise(i + 51) * 0.38f;
        const float am = 0.70f + 0.30f * std::sin(2.0f * kPi * 24.0f * t);

        return (whistle + noise) * am;
    });
    return loadWave(wave);
}

Sound makeBrakeScreechShot()
{
    const float duration = 0.55f;
    const int sampleCount = static_cast<int>(duration * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [=](int i, float t) {
        const float progress = t / duration;
        const float env = std::sin(kPi * progress) * std::exp(-t * 2.2f);
        const float freq = 2100.0f - progress * 1300.0f;
        const float tone = std::sin(2.0f * kPi * freq * t);
        const float noise = whiteNoise(i + 5) * 0.70f;

        return (tone * 0.35f + noise * 0.65f) * env;
    });
    return loadWave(wave);
}

Sound makeImpactShot()
{
    const float duration = 0.32f;
    const int sampleCount = static_cast<int>(duration * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int i, float t) {
        const float env = std::exp(-t * 10.0f);
        const float thump = std::sin(2.0f * kPi * 85.0f * t) * std::exp(-t * 18.0f);
        const float crack = whiteNoise(i + 11) * std::exp(-t * 22.0f) * 0.55f;

        return (thump * 0.75f + crack) * env;
    });
    return loadWave(wave);
}

Sound makeCountdownBeep()
{
    const int sampleCount = static_cast<int>(0.11f * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int /*i*/, float t) {
        const float env = std::exp(-t * 20.0f);

        return std::sin(2.0f * kPi * 880.0f * t) * env * 0.85f;
    });
    return loadWave(wave);
}

Sound makeGoSignal()
{
    const int sampleCount = static_cast<int>(0.65f * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int /*i*/, float t) {
        const float env = std::exp(-t * 3.5f);
        const float rev = std::sin(2.0f * kPi * (120.0f + t * 180.0f) * t) * 0.35f;
        const float horn = std::sin(2.0f * kPi * 523.0f * std::max(0.0f, t - 0.04f))
            + std::sin(2.0f * kPi * 784.0f * std::max(0.0f, t - 0.04f)) * 0.7f;

        return (rev + horn * 0.45f) * env;
    });
    return loadWave(wave);
}

Sound makeVictoryFanfare()
{
    const int sampleCount = static_cast<int>(1.3f * static_cast<float>(kSampleRate));

    Wave wave = makeWaveBuffer(sampleCount, [](int /*i*/, float t) {
        const float env = std::exp(-t * 1.5f);
        const float n1 = std::sin(2.0f * kPi * 523.0f * t);
        const float n2 = std::sin(2.0f * kPi * 659.0f * std::max(0.0f, t - 0.10f));
        const float n3 = std::sin(2.0f * kPi * 784.0f * std::max(0.0f, t - 0.22f));

        return (n1 * 0.38f + n2 * 0.34f + n3 * 0.30f) * env;
    });
    return loadWave(wave);
}

} // namespace

AudioFeedback::AudioFeedback()
{
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }
    if (!IsAudioDeviceReady()) {
        return;
    }
    countdownBeep_ = makeCountdownBeep();
    goSignal_ = makeGoSignal();
    victoryFanfare_ = makeVictoryFanfare();
    engineExhaust_ = makeEngineExhaustLoop();
    engineRasp_ = makeEngineRaspLoop();
    intakeWhoosh_ = makeIntakeLoop();
    driftScreech_ = makeDriftScreechLoop();
    offroadRumble_ = makeOffroadLoop();
    nitroWhistle_ = makeNitroWhistleLoop();
    brakeScreech_ = makeBrakeScreechShot();
    impactThump_ = makeImpactShot();
    ready_ = true;
}

AudioFeedback::~AudioFeedback()
{
    if (!ready_) {
        return;
    }
    stopDrivingLoops();
    UnloadSound(countdownBeep_);
    UnloadSound(goSignal_);
    UnloadSound(victoryFanfare_);
    UnloadSound(engineExhaust_);
    UnloadSound(engineRasp_);
    UnloadSound(intakeWhoosh_);
    UnloadSound(driftScreech_);
    UnloadSound(offroadRumble_);
    UnloadSound(nitroWhistle_);
    UnloadSound(brakeScreech_);
    UnloadSound(impactThump_);
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
}

void AudioFeedback::stopDrivingLoops()
{
    if (!ready_) {
        return;
    }
    StopSound(engineExhaust_);
    StopSound(engineRasp_);
    StopSound(intakeWhoosh_);
    StopSound(driftScreech_);
    StopSound(offroadRumble_);
    StopSound(nitroWhistle_);
}

void AudioFeedback::setLoop(Sound &sound, float volume, float pitch)
{
    if (volume < 0.006f) {
        if (IsSoundPlaying(sound)) {
            StopSound(sound);
        }
        return;
    }
    SetSoundVolume(sound, std::clamp(volume, 0.0f, 1.0f));
    SetSoundPitch(sound, std::clamp(pitch, 0.35f, 2.4f));
    if (!IsSoundPlaying(sound)) {
        PlaySound(sound);
    }
}

void AudioFeedback::updateEngineRpm(const AudioDriveSnapshot &drive, float dt)
{
    const float absSpeed = std::fabs(drive.speed);
    const float speedNorm = std::clamp(absSpeed / std::max(drive.maxSpeed, 1.0f),
        0.0f, 1.0f);
    const bool accelerating = drive.input.throttle > 0.05f;
    const bool braking = drive.input.throttle < -0.05f && absSpeed > 1.5f;

    float rpmFromSpeed = 820.0f + speedNorm * 5400.0f;

    if (accelerating) {
        rpmFromSpeed += drive.input.throttle * (1600.0f + (1.0f - speedNorm) * 900.0f);
    }
    if (braking) {
        rpmFromSpeed -= std::fabs(drive.input.throttle) * 650.0f;
    }
    if (!accelerating && !braking && absSpeed < 2.0f) {
        rpmFromSpeed = 780.0f + speedNorm * 250.0f;
    }
    if (drive.nitroActive) {
        rpmFromSpeed += 850.0f;
    }

    targetRpm_ = std::clamp(rpmFromSpeed, 720.0f, 7400.0f);

    const float revRate = targetRpm_ > engineRpm_
        ? (accelerating ? 14.0f : 8.0f)
        : (braking ? 10.0f : 5.5f);

    engineRpm_ += (targetRpm_ - engineRpm_) * std::min(1.0f, dt * revRate);
}

void AudioFeedback::updateRaceEvents(const RaceState &race,
    const AudioDriveSnapshot &drive)
{
    const RacePhase phase = race.phase();
    const float absSpeed = std::fabs(drive.speed);

    if (phase == RacePhase::COUNTDOWN) {
        const int digit = std::clamp(
            static_cast<int>(std::ceil(race.countdownRemaining())), 1, 3);

        if (digit != lastCountdownDigit_) {
            PlaySound(countdownBeep_);
            lastCountdownDigit_ = digit;
        }
    } else if (lastPhase_ == RacePhase::COUNTDOWN && phase == RacePhase::RACING) {
        PlaySound(goSignal_);
    }

    const bool hardBraking = drive.input.throttle < -0.35f && absSpeed > 8.0f;

    if (hardBraking && !wasHardBraking_ && absSpeed > 12.0f) {
        SetSoundVolume(brakeScreech_, std::clamp(0.35f + absSpeed / 35.0f, 0.35f, 1.0f));
        SetSoundPitch(brakeScreech_, 0.85f + absSpeed / 80.0f);
        PlaySound(brakeScreech_);
    }
    wasHardBraking_ = hardBraking;

    const float speedDrop = lastSpeed_ - drive.speed;

    if (speedDrop > 5.0f && std::fabs(lastSpeed_) > 12.0f && absSpeed > 2.0f) {
        SetSoundVolume(impactThump_, std::clamp(speedDrop / 16.0f, 0.30f, 0.95f));
        PlaySound(impactThump_);
    }

    if (phase == RacePhase::FINISHED && !victoryPlayed_) {
        if (race.playerPosition() == 1) {
            PlaySound(victoryFanfare_);
        }
        victoryPlayed_ = true;
    }
    if (phase == RacePhase::COUNTDOWN) {
        victoryPlayed_ = false;
    }

    lastSpeed_ = drive.speed;
    lastPhase_ = phase;
}

void AudioFeedback::updateDrivingMix(const AudioDriveSnapshot &drive, float dt)
{
    const float absSpeed = std::fabs(drive.speed);
    const float speedNorm = std::clamp(absSpeed / std::max(drive.maxSpeed, 1.0f),
        0.0f, 1.0f);
    const bool accelerating = drive.input.throttle > 0.05f;
    const bool braking = drive.input.throttle < -0.05f && absSpeed > 1.5f;
    const bool offroad = drive.surfaceDrag > 1.5f && absSpeed > 3.0f;

    const float slip = std::fabs(
        normalizeAngle(drive.heading - drive.velocityHeading));
    const float driftIntensity = drive.drifting
        ? std::clamp(speedNorm * (0.40f + slip / kPi), 0.0f, 1.0f)
        : 0.0f;

    const float rpmNorm = std::clamp(
        (engineRpm_ - 700.0f) / 6700.0f, 0.0f, 1.0f);
    const float pitch = engineRpm_ / kRefEngineRpm;

    float load = 0.18f + rpmNorm * 0.42f;

    if (accelerating) {
        load += drive.input.throttle * 0.38f;
    } else if (braking) {
        load *= 0.55f;
    } else if (absSpeed < 1.0f) {
        load = 0.14f;
    }

    const float exhaustTarget = std::clamp(0.08f + load * 0.52f, 0.0f, 0.72f);
    const float raspTarget = std::clamp(rpmNorm * 0.38f + load * 0.14f, 0.0f, 0.48f);
    const float intakeTarget = accelerating
        ? std::clamp(drive.input.throttle * (0.18f + speedNorm * 0.28f), 0.0f, 0.42f)
        : 0.0f;
    const float driftTarget = driftIntensity * 0.58f;
    const float offroadTarget = offroad ? (0.12f + speedNorm * 0.34f) : 0.0f;
    const float nitroTarget = drive.nitroActive
        ? std::clamp(0.22f + speedNorm * 0.30f, 0.0f, 0.55f)
        : 0.0f;

    const float blend = std::min(1.0f, dt * 16.0f);

    exhaustVol_ += (exhaustTarget - exhaustVol_) * blend;
    raspVol_ += (raspTarget - raspVol_) * blend;
    intakeVol_ += (intakeTarget - intakeVol_) * blend;
    driftVol_ += (driftTarget - driftVol_) * blend;
    offroadVol_ += (offroadTarget - offroadVol_) * blend;
    nitroVol_ += (nitroTarget - nitroVol_) * blend;

    setLoop(engineExhaust_, exhaustVol_, pitch);
    setLoop(engineRasp_, raspVol_, pitch * 1.03f);
    setLoop(intakeWhoosh_, intakeVol_, pitch * 0.92f);
    setLoop(driftScreech_, driftVol_, 0.75f + speedNorm * 0.65f);
    setLoop(offroadRumble_, offroadVol_, 0.70f + speedNorm * 0.40f);
    setLoop(nitroWhistle_, nitroVol_, 1.05f + speedNorm * 0.55f);
}

void AudioFeedback::updateOpenWorld(const AudioDriveSnapshot &drive, float dt,
    racer::world::BiomeId biome, bool paused)
{
    if (!ready_) {
        return;
    }
    if (paused) {
        stopDrivingLoops();
        return;
    }
    updateEngineRpm(drive, dt);
    updateDrivingMix(drive, dt);

    float regional = 1.0f;
    switch (biome) {
    case racer::world::BiomeId::FOREST:
        regional = 0.82f;
        break;
    case racer::world::BiomeId::PORT:
        regional = 0.95f;
        break;
    case racer::world::BiomeId::VOLCANO:
        regional = 0.88f;
        break;
    default:
        break;
    }
    setLoop(engineExhaust_, exhaustVol_ * regional, engineRpm_ / kRefEngineRpm);
}

void AudioFeedback::update(const RaceState &race, const AudioDriveSnapshot &drive,
    float dt, bool paused)
{
    if (!ready_) {
        return;
    }
    if (paused) {
        stopDrivingLoops();
        return;
    }

    updateRaceEvents(race, drive);

    const RacePhase phase = race.phase();

    if (phase == RacePhase::RACING || phase == RacePhase::WRAP_UP) {
        updateEngineRpm(drive, dt);
        updateDrivingMix(drive, dt);
    } else {
        stopDrivingLoops();
        engineRpm_ = 820.0f;
        targetRpm_ = 820.0f;
        exhaustVol_ = 0.0f;
        raspVol_ = 0.0f;
        intakeVol_ = 0.0f;
        driftVol_ = 0.0f;
        offroadVol_ = 0.0f;
        nitroVol_ = 0.0f;
    }
}

} // namespace app
} // namespace racer
