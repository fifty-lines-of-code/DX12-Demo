#pragma once

#include <xaudio2.h>
#include <string>

namespace Engine::EngineAudio {

    struct AudioBufferAsset {
        // the file path
        std::wstring filePath = L"";

        // Raw binary byte storage
        uint8_t* rawPCMBytes = nullptr;
        uint32_t dataSize = 0;

        // Parsed format detailing channels, 
        // bit depth, and sample rate
        WAVEFORMATEX waveFormat = {};

        // Playback hardware abstraction handle
        IXAudio2SourceVoice* sourceVoice = nullptr;

        // total duration
        float totalDurationSeconds = 0.0f;

        // easily identify if it's loaded or not
        bool isLoaded = false;
    };
}