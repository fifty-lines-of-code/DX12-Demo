#pragma once

#include "AudioBufferAsset.h"
#include <cstdint>
#include <fstream>
#include "../../../Helper/Logger.h"

namespace Engine::EngineAudio {

    class AudioManagerHelper {
    public:
        static constexpr float MAX_VOLUME = 0.7f;
        static constexpr float MIN_VOLUME = 0.0f;
        static constexpr float BG_AUDIO_DUCKED_VOLUME = 0.6f;
        // 50 milliseconds for ramp up/down
        static constexpr float TRANSITION_WINDOW = 0.05f;
    };
}