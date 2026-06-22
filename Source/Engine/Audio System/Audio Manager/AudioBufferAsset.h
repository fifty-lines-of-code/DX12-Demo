#pragma once

#include <xaudio2.h>
#include <string>

namespace Engine::EngineAudio {

    struct AudioBufferAsset {
        std::wstring filePath = L"";

        // Raw binary byte storage
        uint8_t* rawPCMBytes = nullptr;
        uint32_t dataSize = 0;

        // Parsed format detailing channels, bit depth, and sample rate
        WAVEFORMATEX waveFormat = {};

        // Playback hardware abstraction handle
        IXAudio2SourceVoice* sourceVoice = nullptr;
        // total duration
        float totalDurationSeconds = 0.0f;

        bool isLoaded = false;
    };
}