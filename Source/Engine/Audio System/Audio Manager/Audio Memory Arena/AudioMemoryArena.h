#pragma once

#include <array>
#include "../AudioBufferAsset.h"
#include <cstdint>
#include <xaudio2.h>

namespace Engine::EngineAudio {

    class AudioMemoryArena {
    public:
        AudioMemoryArena() = default;
        ~AudioMemoryArena() = default;

        bool LoadAssets(
            AudioBufferAsset* assets,
            uint32_t count,
            IXAudio2* xAudioEngine
        );

    private:
        // 32 MB Pool tracked via standard 32-bit unsigned integers
        static constexpr uint32_t AUDIO_ARENA_SIZE_BYTES = 1024 * 1024 * 32;

        uint8_t mMemoryArena[AUDIO_ARENA_SIZE_BYTES] = { 0 };
        uint32_t mArenaOffset = 0;

    private:
        bool LoadWavIntoArena(
            AudioBufferAsset& outAsset,
            IXAudio2* xAudioEngine
        );
        uint8_t* Allocate(uint32_t size);
    };
}