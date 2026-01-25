#pragma once

namespace Game::Component {

struct EngineAudioComponent {
    std::string soundName = "engine_low";
    float minPitch = 0.6f;
    float maxPitch = 1.0f;
    float smoothingAlpha = 0.15f; // 0..1, higher = faster response
    float currentPitch = minPitch; // runtime smoothing state
    bool autoPlay = true;
};

} // namespace Game::Component
