#include "AudioManager.h"

#include "AudioManagerHelper.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace Engine::EngineAudio {
    
    AudioManager::AudioManager() :
        mCurrentlyPlayingBGAudio(SoundBG::INVALID) 
    {
        // Spin up low-level API engine instance
        HRESULT hr = XAudio2Create(&mXAudioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (SUCCEEDED(hr)) {
            mXAudioEngine->CreateMasteringVoice(&mMasteringVoice);
        }
    }

    AudioManager::~AudioManager() {
        StopBackgroundLoop();

        // Clean up allocated raw audio bytes
        for (auto& track : mBGLibrary) {
            if (track.sourceVoice) {
                track.sourceVoice->DestroyVoice();
            }
            delete[] track.rawPCMBytes;
        }
        for (auto& sfx : mSFXLibrary) {
            if (sfx.sourceVoice) {
                sfx.sourceVoice->DestroyVoice();
            }
            delete[] sfx.rawPCMBytes;
        }

        if (mMasteringVoice) {
            mMasteringVoice->DestroyVoice();
        }
        if (mXAudioEngine) {
            mXAudioEngine->Release();
        }
    }

    bool AudioManager::Initialize() {
        if (!mXAudioEngine) { return false; }

        const std::filesystem::path audioDir = L"Source/Resources/Audio";
        mBGLibrary[(uint8_t)SoundBG::BOSSFIGHT].filePath = audioDir / L"Background/bg-1.wav";
        mSFXLibrary[(uint8_t)SoundSFX::LIGHT_ATTACK].filePath = audioDir / L"SFX/light_attack.wav";

        // TODO: spin up background thread for loading of bg and sfx audio
        // and load per level

        // Process BG assets loading
        for (auto& track : mBGLibrary) {
            if (!AudioManagerHelper::LoadWavFileRaw(track.filePath, track)) {
                return false;
            }
            HRESULT hr = mXAudioEngine->CreateSourceVoice(
                &track.sourceVoice,
                &track.waveFormat
            );
            track.isLoaded = SUCCEEDED(hr);
        }

        // Process SFX assets loading
        for (auto& sfx : mSFXLibrary) {
            if (!AudioManagerHelper::LoadWavFileRaw(sfx.filePath, sfx)) {
                return false;
            }
            HRESULT hr = mXAudioEngine->CreateSourceVoice(
                &sfx.sourceVoice,
                &sfx.waveFormat
            );
            sfx.isLoaded = SUCCEEDED(hr);
        }

        return true;
    }

    void AudioManager::Update(float deltaTime) {
        // todo:
    }

    void AudioManager::StartBackgroundLoop(SoundBG track) {
        if (mCurrentlyPlayingBGAudio == track) { return; }

        StopBackgroundLoop();

        uint8_t index = (uint8_t)track;
        if (index < mBGLibrary.size() && mBGLibrary[index].isLoaded) {
            AudioBufferAsset& asset = mBGLibrary[index];

            XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<UINT32>(asset.dataSize);
            buffer.pAudioData = asset.rawPCMBytes; // Raw Pointer
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE; // Infinite loop flag
            buffer.Flags = XAUDIO2_END_OF_STREAM;

            mCurrentLoopVoice = asset.sourceVoice;
            mCurrentLoopVoice->SubmitSourceBuffer(&buffer);
            mCurrentLoopVoice->Start(0);
            mCurrentlyPlayingBGAudio = track;
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
        size_t index = static_cast<size_t>(sfx);
        if (index < mSFXLibrary.size() && mSFXLibrary[index].isLoaded) {
            auto& target = mSFXLibrary[index];

            XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<UINT32>(target.dataSize);
            buffer.pAudioData = target.rawPCMBytes; // Raw Pointer
            buffer.Flags = XAUDIO2_END_OF_STREAM; // just once

            // One-shots must flush old buffers in case it is hit rapidly 
            target.sourceVoice->Stop(0);
            target.sourceVoice->FlushSourceBuffers();
            target.sourceVoice->SubmitSourceBuffer(&buffer);
            target.sourceVoice->Start(0);
        }
    }
}