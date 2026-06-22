#include "AudioManager.h"

#include "AudioManagerHelper.h"
#include <filesystem>
#include "../../../Helper/Helper.h"
#include "../../../Helper/Logger.h"

namespace Engine::EngineAudio {

    AudioManager::AudioManager() :
        mActiveSFXType(SoundSFX::INVALID)
    {
        if (SUCCEEDED(XAudio2Create(&mXAudioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
            mXAudioEngine->CreateMasteringVoice(&mMasteringVoice);
        }
    }

    AudioManager::~AudioManager() {
        StopBackgroundLoop();

        for (auto& track : mBGLibrary) {
            if (track.sourceVoice) track.sourceVoice->DestroyVoice();
        }
        for (auto& sfx : mSFXLibrary) { 
            if (sfx.sourceVoice) sfx.sourceVoice->DestroyVoice(); 
        }

        if (mMasteringVoice) {
            mMasteringVoice->DestroyVoice();
        }

        if (mXAudioEngine) {
            mXAudioEngine->Release();
        }
    }

    bool AudioManager::Initialize() {
        if (!mXAudioEngine) return false;

        const std::filesystem::path audioDir = L"Source/Resources/Audio";

        // 1. Assign target resource file locations
        mBGLibrary[static_cast<size_t>(SoundBG::BOSSFIGHT)].filePath = audioDir / L"Background/bg-1.wav";
        mSFXLibrary[static_cast<size_t>(SoundSFX::LIGHT_ATTACK)].filePath = audioDir / L"SFX/light_attack.wav";

        // 2. Offload background tracks array loading to the arena
        if (!mMemoryArena.LoadAssets(mBGLibrary.data(), mBGLibrary.size(), mXAudioEngine)) {
            // we only fail here if we ran into arena overflow
            return false;
        }

        // 3. Offload SFX tracks array loading to the arena
        if (!mMemoryArena.LoadAssets(mSFXLibrary.data(), mSFXLibrary.size(), mXAudioEngine)) {
            // we only fail here if we ran into arena overflow
            return false;
        }

        return true;
    }

    void AudioManager::Update(float deltaTime) {
        DuckBackgroundAudioIfSFXPlaying(deltaTime);
    }

    void AudioManager::StartBackgroundLoop(SoundBG track) {
        if (IsPlayingSoundBG(track) ||
            track == SoundBG::INVALID)
        { return; }

        StopBackgroundLoop();

        size_t idx = static_cast<size_t>(track);

        if (idx < mBGLibrary.size() && mBGLibrary[idx].isLoaded) {
            auto& target = mBGLibrary[idx];
            XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<UINT32>(target.dataSize);
            buffer.pAudioData = target.rawPCMBytes;
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
            buffer.Flags = XAUDIO2_END_OF_STREAM;

            mCurrentLoopVoice = target.sourceVoice;
            mCurrentLoopVoice->SubmitSourceBuffer(&buffer);
            mCurrentLoopVoice->Start(0);
        }
    }

    void AudioManager::StopBackgroundLoop() {
        if (mCurrentLoopVoice) {
            mCurrentLoopVoice->Stop(0);
            mCurrentLoopVoice->FlushSourceBuffers();
            mCurrentLoopVoice = nullptr;
        }
    }

    void AudioManager::PlayOneShot(SoundSFX sfx) {
        if (IsPlayingSoundSFX(sfx)) { return; }

        size_t idx = static_cast<size_t>(sfx);

        if (idx < mSFXLibrary.size() && mSFXLibrary[idx].isLoaded) {
            auto& target = mSFXLibrary[idx];
            XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<UINT32>(target.dataSize);
            buffer.pAudioData = target.rawPCMBytes;
            buffer.Flags = XAUDIO2_END_OF_STREAM;

            target.sourceVoice->Stop(0);
            target.sourceVoice->FlushSourceBuffers();

            // Start SFX completely silent (Update will ramp it up)
            target.sourceVoice->SetVolume(0.0f);

            target.sourceVoice->SubmitSourceBuffer(&buffer);
            target.sourceVoice->Start(0);

            // Arm the tracking system
            mIsSFXActive = true;
            mActiveSFXType = sfx;
            mSFXElapsedTime = 0.0f;
        }
    }

    bool AudioManager::IsPlayingSoundBG(SoundBG bg) const {
        size_t idx = static_cast<size_t>(bg);
        if (idx >= mBGLibrary.size() || !mBGLibrary[idx].isLoaded) {
            return false;
        }

        return IsPlayingSound(mBGLibrary[idx]);
    }

    bool AudioManager::IsPlayingSoundSFX(SoundSFX sfx) const {
        size_t idx = static_cast<size_t>(sfx);
        if (idx >= mSFXLibrary.size() || !mSFXLibrary[idx].isLoaded) {
            return false;
        }

        return IsPlayingSound(mSFXLibrary[idx]);
    }

    bool AudioManager::IsPlayingSound(const AudioBufferAsset& asset) const {
        XAUDIO2_VOICE_STATE state;
        // Tell XAudio2 to populate the state structure
        asset.sourceVoice->GetState(&state);

        // If BuffersQueued is greater than 0, the sound is still processing
        return (state.BuffersQueued > 0);
    }

    void AudioManager::DuckBackgroundAudioIfSFXPlaying(float deltaTime) {
        if (!mIsSFXActive || !IsPlayingSoundSFX(mActiveSFXType)) {
            mIsSFXActive = false;
            // Default idle state: ensure background music is resting at full volume
            if (mCurrentLoopVoice) {
                mCurrentLoopVoice->SetVolume(AudioManagerHelper::MAX_VOLUME);
            }
            return;
        }

        // update the elapsed time
        mSFXElapsedTime += deltaTime;

        const auto& sfxAsset = mSFXLibrary[(uint8_t)mActiveSFXType];
        float totalDuration = sfxAsset.totalDurationSeconds;

        float targetSFXVol;
        float targetBGVol;

        // the intro (TRANSITION_WINDOW) ramp
        if (mSFXElapsedTime <= AudioManagerHelper::TRANSITION_WINDOW) {
            float alpha = mSFXElapsedTime / AudioManagerHelper::TRANSITION_WINDOW;

            targetSFXVol = Helper::Lerp(
                AudioManagerHelper::MIN_VOLUME,
                AudioManagerHelper::MAX_VOLUME,
                alpha
            );
            targetBGVol = Helper::Lerp(
                AudioManagerHelper::MAX_VOLUME,
                AudioManagerHelper::BG_AUDIO_DUCKED_VOLUME,
                alpha
            );
        }
        // the outro (TRANSITION_WINDOW) ramp
        else if (mSFXElapsedTime >= (totalDuration - AudioManagerHelper::TRANSITION_WINDOW)) {
            float timeIntoOutro = mSFXElapsedTime - (totalDuration - AudioManagerHelper::TRANSITION_WINDOW);
            float alpha = timeIntoOutro / AudioManagerHelper::TRANSITION_WINDOW;

            targetSFXVol = Helper::Lerp(
                AudioManagerHelper::MAX_VOLUME,
                AudioManagerHelper::MIN_VOLUME,
                alpha
            );

            targetBGVol = Helper::Lerp(
                AudioManagerHelper::BG_AUDIO_DUCKED_VOLUME,
                AudioManagerHelper::MAX_VOLUME,
                alpha
            );
        }
        // sustained playback within the playback window
        else {
            targetSFXVol = AudioManagerHelper::MAX_VOLUME;
            targetBGVol = AudioManagerHelper::BG_AUDIO_DUCKED_VOLUME;
        }

        sfxAsset.sourceVoice->SetVolume(targetSFXVol);
        if (mCurrentLoopVoice) {
            mCurrentLoopVoice->SetVolume(targetBGVol);
        }
    }
}