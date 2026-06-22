#include "AudioManager.h"

#include <filesystem>
#include "../../../Helper/Logger.h"

namespace Engine::EngineAudio {

    AudioManager::AudioManager() :
        mCurrentlyPlayingBGAudio(SoundBG::INVALID)
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
        // todo:
    }

    void AudioManager::StartBackgroundLoop(SoundBG track) {
        if (mCurrentlyPlayingBGAudio == track ||
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
            mCurrentlyPlayingBGAudio = track;
        }
        else {
            mCurrentlyPlayingBGAudio = SoundBG::INVALID;
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
        size_t idx = static_cast<size_t>(sfx);

        if (idx < mSFXLibrary.size() && mSFXLibrary[idx].isLoaded) {
            auto& target = mSFXLibrary[idx];
            XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<UINT32>(target.dataSize);
            buffer.pAudioData = target.rawPCMBytes;
            buffer.Flags = XAUDIO2_END_OF_STREAM;

            target.sourceVoice->Stop(0);
            target.sourceVoice->FlushSourceBuffers();
            target.sourceVoice->SubmitSourceBuffer(&buffer);
            target.sourceVoice->Start(0);
        }
    }
}