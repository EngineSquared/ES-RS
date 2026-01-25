#include "EngineAudioSystem.hpp"

#include "Physics.hpp"
#include "Sound.hpp"
#include "Logger.hpp"
#include <fmt/format.h>

static float ComputePitchClamped(float rpm, float minPitch, float maxPitch, float minRPM, float maxRPM)
{
    if (maxRPM <= minRPM)
        return minPitch;
    float t = (rpm - minRPM) / (maxRPM - minRPM);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return minPitch + t * (maxPitch - minPitch);
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

        float rpm = rpmOpt.has_value() ? rpmOpt.value() : 0.0f;
        float targetPitch = ComputePitchClamped(rpm, audio.minPitch, audio.maxPitch, vehicle.engine.minRPM, vehicle.engine.maxRPM);
        float alpha = audio.smoothingAlpha;
        audio.currentPitch = alpha * targetPitch + (1.0f - alpha) * audio.currentPitch;

        Log::Info(fmt::format("Engine RPM: {:.1f}, Pitch: {:.2f}", rpm, audio.currentPitch));

        sound.SetPitch(audio.soundName, audio.currentPitch);

        if (audio.autoPlay) {
            if (rpm > vehicle.engine.minRPM && !sound.IsPlaying(audio.soundName)) {
                Log::Info(fmt::format("Starting engine sound: {}", audio.soundName));
                sound.Play(audio.soundName);
            } else if (rpm < 1.0f && sound.IsPlaying(audio.soundName)) {
                Log::Info(fmt::format("Stopping engine sound: {}", audio.soundName));
                sound.Stop(audio.soundName);
            }
        }
    }
}
