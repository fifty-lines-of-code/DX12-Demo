#include "AudioSystem.h"

namespace Engine::EngineAudio {

    bool AudioSystem::Initialize() {
        return mAudioManager.Initialize();
    }

    void AudioSystem::Update(float deltaTime) {
        mAudioManager.Update(deltaTime);
    }

    void AudioSystem::StartBackgroundLoop(SoundBG track) {
        mAudioManager.StartBackgroundLoop(track);
    }

    void AudioSystem::PlayOneShot(SoundSFX sfx) {
        mAudioManager.PlayOneShot(sfx);
    }

    void AudioSystem::StopBackgroundLoop() {
        mAudioManager.StopBackgroundLoop();
    }
}