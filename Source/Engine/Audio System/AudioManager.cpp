#include "AudioManager.h"

#include "../../Helper/Logger.h"
#include <iostream>

namespace Engine::EngineAudio {

    // Helper to safely format exceptions into wide strings for your Logger
    namespace {
        std::wstring ToWideString(std::string_view narrowStr) {
            return std::wstring(narrowStr.begin(), narrowStr.end());
        }
    }

    AudioManager::AudioManager() {
        DirectX::AUDIO_ENGINE_FLAGS flags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
        flags |= DirectX::AudioEngine_Debug;
#endif

        mAudioEngine = std::make_unique<DirectX::AudioEngine>(flags);
    }

    AudioManager::~AudioManager() {
        if (mCurrentLoopInstance) {
            mCurrentLoopInstance->Stop();
        }
    }

    void AudioManager::Initialize() {
        const std::filesystem::path audioDir = L"Assets/Audio";

        try {
            mBGLibrary[SoundBG::BOSSFIGHT] = std::make_unique<DirectX::SoundEffect>(
                mAudioEngine.get(), (audioDir / L"bossfight.wav").c_str()
            );

            mSFXLibrary[SoundSFX::LIGHT_ATTACK] = std::make_unique<DirectX::SoundEffect>(
                mAudioEngine.get(), (audioDir / L"light_attack.wav").c_str()
            );
        }
        catch (const std::exception& e) {
            std::wstring logMessage = L"Audio Loading failed: " + ToWideString(e.what());
            Logger::ERR(logMessage.c_str());
            return;
        }

        if (auto it = mBGLibrary.find(SoundBG::BOSSFIGHT); it != mBGLibrary.end()) {
            const auto& [enumKey, soundTrack] = *it;
            if (soundTrack) {
                mCurrentLoopInstance = soundTrack->CreateInstance();
                mCurrentLoopInstance->Play(true);
            }
        }
    }

    void AudioManager::Update() {
        if (!mAudioEngine->Update() && mAudioEngine->IsCriticalError()) {
            Logger::ERR(L"Audio device lost! Critical audio hardware error encountered.");
        }
    }

    void AudioManager::PlayOneShot(SoundSFX sfx) {
        if (auto it = mSFXLibrary.find(sfx); it != mSFXLibrary.end()) {
            const auto& [enumKey, soundEffect] = *it;
            if (soundEffect) {
                soundEffect->Play();
            }
        }
    }
}