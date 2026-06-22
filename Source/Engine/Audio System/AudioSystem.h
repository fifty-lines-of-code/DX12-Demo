#pragma once

#include "Audio Manager/AudioManager.h"

namespace Engine::EngineAudio {

    class AudioSystem {
    public:
        AudioSystem() = default;
        ~AudioSystem() = default;

        AudioSystem(const AudioSystem&) = delete;
        AudioSystem& operator=(const AudioSystem&) = delete;

        bool Initialize();
        void Update(float deltaTime);

        void StartBackgroundLoop(SoundBG track);
        void PlayOneShot(SoundSFX sfx);
        void StopBackgroundLoop();

    private:
        AudioManager mAudioManager;
    };
}