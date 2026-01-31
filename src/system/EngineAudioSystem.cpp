#include "EngineAudioSystem.hpp"

#include "Physics.hpp"
#include "Sound.hpp"
#include "Logger.hpp"
#include <fmt/format.h>
#include <cmath>

struct BlendWeights {
    float low;
    float mid;
    float high;
};

static BlendWeights ComputeBlendWeights(float rpm, float minRPM, float maxRPM)
{
    BlendWeights weights{0.0f, 0.0f, 0.0f};

    if (maxRPM <= minRPM) {
        weights.low = 1.0f;
        return weights;
    }

    // Normalize RPM to 0..1 range
    float t = (rpm - minRPM) / (maxRPM - minRPM);
    t = std::clamp(t, 0.0f, 1.0f);

    // Define blend ranges with overlap for smooth transitions
    // Low:  0.0 -> 0.5 (full at 0.0, fades to 0 by 0.5)
    // Mid:  0.3 -> 0.7 (fades in at 0.3, full at 0.4-0.6, fades out by 0.7)
    // High: 0.5 -> 1.0 (fades in at 0.5, full at 1.0)

    if (t <= 0.3f) {
        // Low range: fully low, starting to fade in mid
        weights.low = 1.0f;
        weights.mid = 0.0f;
    } else if (t <= 0.5f) {
        // Low-to-mid transition using linear interpolation
        float fade = (t - 0.3f) / 0.2f; // Normalize to 0..1 over the 0.3-0.5 range
        weights.low = std::lerp(1.0f, 0.0f, fade);
        weights.mid = std::lerp(0.0f, 1.0f, fade);
    } else if (t <= 0.7f) {
        // Mid-to-high transition using linear interpolation
        float fade = (t - 0.5f) / 0.2f; // Normalize to 0..1 over the 0.5-0.7 range
        weights.mid = std::lerp(1.0f, 0.0f, fade);
        weights.high = std::lerp(0.0f, 1.0f, fade);
    } else {
        // High range: fully high
        weights.mid = 0.0f;
        weights.high = 1.0f;
    }

    return weights;
}

void EngineAudioSystem(Engine::Core &core)
{
    auto &registry = core.GetRegistry();
    auto &telemetry = core.GetResource<Physics::Resource::VehicleTelemetry>();
    auto &sound = core.GetResource<Sound::Resource::SoundManager>();

    auto view = registry.view<Game::Component::EngineAudioComponent, Physics::Component::Vehicle>();
    for (auto [e, audio, vehicle] : view.each())
    {
        Engine::EntityId eid{static_cast<Engine::EntityId::ValueType>(e)};
        auto rpmOpt = telemetry.GetRPM(eid);

        if (!rpmOpt.has_value()) {
            continue;
        }

        float rpm = rpmOpt.value();

        // Compute target blend weights for the three samples
        BlendWeights targetWeights = ComputeBlendWeights(rpm, vehicle.engine.minRPM, vehicle.engine.maxRPM);

        // Apply smoothing to volume changes using linear interpolation
        float alpha = audio.smoothingAlpha;
        audio.currentVolumeLow = std::lerp(audio.currentVolumeLow, targetWeights.low, alpha);
        audio.currentVolumeMid = std::lerp(audio.currentVolumeMid, targetWeights.mid, alpha);
        audio.currentVolumeHigh = std::lerp(audio.currentVolumeHigh, targetWeights.high, alpha);

        /* Log::Info(fmt::format("Engine RPM: {:.1f}, Pitch: {:.2f}", rpm, audio.currentPitch)); */

        // Set volumes for all three samples
        sound.SetVolume(audio.soundNameLow, audio.currentVolumeLow);
        sound.SetVolume(audio.soundNameMid, audio.currentVolumeMid);
        sound.SetVolume(audio.soundNameHigh, audio.currentVolumeHigh);

        if (audio.autoPlay) {
            bool shouldPlay = rpm > vehicle.engine.minRPM;

            // Start/stop all three samples together
            if (shouldPlay) {
                if (!sound.IsPlaying(audio.soundNameLow)) {
                    Log::Info("Starting engine sounds (low/mid/high)");
                    sound.Play(audio.soundNameLow);
                    sound.Play(audio.soundNameMid);
                    sound.Play(audio.soundNameHigh);
                }
            } else if (rpm < 1.0f) {
                if (sound.IsPlaying(audio.soundNameLow)) {
                    Log::Info("Stopping engine sounds (low/mid/high)");
                    sound.Stop(audio.soundNameLow);
                    sound.Stop(audio.soundNameMid);
                    sound.Stop(audio.soundNameHigh);
                }
            }
        }
    }
}
