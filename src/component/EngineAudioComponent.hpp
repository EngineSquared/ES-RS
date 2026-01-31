#pragma once

namespace Game::Component {

struct EngineAudioComponent {
    // Three sound samples for low, mid, and high RPM ranges
    std::string soundNameLow = "engine_low";
    std::string soundNameMid = "engine_mid";
    std::string soundNameHigh = "engine_high";

    // Volume smoothing for each sample
    float smoothingAlpha = 0.15f; // 0..1, higher = faster response
    float currentVolumeLow = 0.0f;  // runtime smoothing state
    float currentVolumeMid = 0.0f;
    float currentVolumeHigh = 0.0f;

    bool autoPlay = true;
};

} // namespace Game::Component
