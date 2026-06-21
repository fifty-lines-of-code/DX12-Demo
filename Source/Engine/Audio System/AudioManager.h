#pragma once

#define _USE_MATH_DEFINES
#include <Audio.h>
#include <memory>
#include <string_view>
#include <filesystem>
#include <unordered_map>

namespace Engine::EngineAudio {

    enum class SoundBG : uint8_t {
        BOSSFIGHT
    };

    enum class SoundSFX : uint8_t {
        LIGHT_ATTACK
    };

    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        void Initialize();
        void Update();
        void PlayOneShot(SoundSFX sfx);

    private:
        std::unique_ptr<DirectX::AudioEngine> mAudioEngine;

        std::unordered_map<SoundBG, std::unique_ptr<DirectX::SoundEffect>> mBGLibrary;
        std::unordered_map<SoundSFX, std::unique_ptr<DirectX::SoundEffect>> mSFXLibrary;

        std::unique_ptr<DirectX::SoundEffectInstance> mCurrentLoopInstance;
    };
}