#pragma once

#include <array>
#include "AudioBufferAsset.h"
#include "Audio Memory Arena/AudioMemoryArena.h"
#include <xaudio2.h>

namespace Engine::EngineAudio {

    enum class SoundBG : uint8_t {
        BOSSFIGHT = 0,
        COUNT,
        INVALID
    };

    enum class SoundSFX : uint8_t {
        LIGHT_ATTACK = 0,
        COUNT,
        INVALID
    };

    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        // Prevent copies to safeguard raw pointer tracking pointers
        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        bool Initialize();
        void Update(float deltaTime);
        void StartBackgroundLoop(SoundBG track);
        void StopBackgroundLoop();
        void PlayOneShot(SoundSFX sfx);

    private:
        AudioMemoryArena mMemoryArena;
        // our bg and sfx audio buffers
        std::array<AudioBufferAsset, (uint8_t)SoundBG::COUNT> mBGLibrary;
        std::array<AudioBufferAsset, (uint8_t)SoundSFX::COUNT> mSFXLibrary;

        // Low-level Native XAudio2 Engine Interfaces
        IXAudio2* mXAudioEngine = nullptr;
        IXAudio2MasteringVoice* mMasteringVoice = nullptr;

        // Track state management
        IXAudio2SourceVoice* mCurrentLoopVoice = nullptr;

        bool mIsSFXActive = false;
        SoundSFX mActiveSFXType;
        float mSFXElapsedTime = 0.0f;
        

    private:
        bool IsPlayingSoundBG(SoundBG bg) const;
        bool IsPlayingSoundSFX(SoundSFX sfx) const;
        bool IsPlayingSound(const AudioBufferAsset& asset) const;
        void DuckBackgroundAudioIfSFXPlaying(float deltaTime);
    };
}