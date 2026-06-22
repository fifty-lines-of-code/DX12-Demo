#pragma once

#include "Audio Manager/AudioManager.h"
#include "../Input System/IInputSystem.h"

namespace Engine::EngineAudio {

    class AudioSystem {
    public:
        AudioSystem() = default;
        ~AudioSystem() = default;

        AudioSystem(const AudioSystem&) = delete;
        AudioSystem& operator=(const AudioSystem&) = delete;

        bool Initialize();
        void Update(
            const IInputSystem& inputSystem,
            float deltaTime
        );

        void StartBackgroundLoop(SoundBG track);
        void PlayOneShot(SoundSFX sfx);
        void StopBackgroundLoop();

    private:
        AudioManager mAudioManager;
    };
}