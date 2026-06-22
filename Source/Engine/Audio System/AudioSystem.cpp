#include "AudioSystem.h"

namespace Engine::EngineAudio {

    bool AudioSystem::Initialize() {
        return mAudioManager.Initialize();
    }

    void AudioSystem::Update(
        const IInputSystem& inputSystem,
        float deltaTime) {
        mAudioManager.Update(deltaTime);

        if (inputSystem.GetButtonState(GameButton::BumperRight) == GameButtonState::Just_Pressed) {
            mAudioManager.PlayOneShot(SoundSFX::LIGHT_ATTACK);
        }
    }

    void AudioSystem::StartBackgroundLoop(SoundBG track) {
        mAudioManager.StartBackgroundLoop(track);
    }

    void AudioSystem::StopBackgroundLoop() {
        mAudioManager.StopBackgroundLoop();
    }
}