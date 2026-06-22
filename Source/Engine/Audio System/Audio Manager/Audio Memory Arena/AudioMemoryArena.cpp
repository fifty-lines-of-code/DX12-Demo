#include "AudioMemoryArena.h"

#include <fstream>
#include "../../../../Helper/Logger.h"
#include <string_view>
#include <iostream>

namespace Engine::EngineAudio {

    bool AudioMemoryArena::LoadAssets(
        AudioBufferAsset* assets,
        uint32_t count, 
        IXAudio2* xAudioEngine
    ) {
        for (uint32_t i = 0; i < count; ++i) {
            AudioBufferAsset& asset = assets[i];

            if (!LoadWavIntoArena(asset, xAudioEngine)) {
                if (asset.rawPCMBytes == nullptr && asset.dataSize > 0) {
                    // Arena overflow
                    return false;
                }
                asset.isLoaded = false;
            }
        }
        return true;
    }

#pragma region Private

    bool AudioMemoryArena::LoadWavIntoArena(
        AudioBufferAsset& outAsset, 
        IXAudio2* xAudioEngine
    ) {
        std::ifstream file(outAsset.filePath, std::ios::binary);

        if (!file.is_open()) {
            return false;
        }

        char mainHeader[12];
        if (!file.read(mainHeader, 12)) {
            return false;
        }

        if (std::string_view(mainHeader, 4) != "RIFF" ||
            std::string_view(mainHeader + 8, 4) != "WAVE") {
            return false;
        }

        char chunkId[4];
        uint32_t chunkSize = 0;
        bool foundFmt = false;
        bool foundData = false;

        while (file.read(chunkId, 4) && file.read(reinterpret_cast<char*>(&chunkSize), 4)) {
            std::string_view currentChunk(chunkId, 4);

            if (currentChunk == "fmt ") {
                uint32_t bytesToRead = (chunkSize > sizeof(WAVEFORMATEX)) ? sizeof(WAVEFORMATEX) : chunkSize;
                file.read(reinterpret_cast<char*>(&outAsset.waveFormat), bytesToRead);
                if (chunkSize > bytesToRead) file.seekg(chunkSize - bytesToRead, std::ios::cur);
                outAsset.waveFormat.cbSize = 0;
                foundFmt = true;
            }
            else if (currentChunk == "data") {
                outAsset.dataSize = chunkSize;
                outAsset.rawPCMBytes = Allocate(chunkSize);

                if (outAsset.rawPCMBytes == nullptr) {
                    file.close();
                    // Arena overflow
                    return false; 
                }

                file.read(reinterpret_cast<char*>(outAsset.rawPCMBytes), chunkSize);
                foundData = true;

                if (outAsset.waveFormat.nAvgBytesPerSec > 0) {
                    outAsset.totalDurationSeconds = static_cast<float>(outAsset.dataSize) / static_cast<float>(outAsset.waveFormat.nAvgBytesPerSec);
                }
                break;
            }
            else {
                uint32_t alignmentPadding = chunkSize % 2;
                file.seekg(chunkSize + alignmentPadding, std::ios::cur);
            }
        }
        file.close();

        if (foundFmt && foundData && outAsset.waveFormat.wFormatTag == 1) {
            HRESULT hr = xAudioEngine->CreateSourceVoice(&outAsset.sourceVoice, &outAsset.waveFormat);
            outAsset.isLoaded = SUCCEEDED(hr);
            return outAsset.isLoaded;
        }

        return false;
    }

    uint8_t* AudioMemoryArena::Allocate(uint32_t size) {
        if (mArenaOffset + size > AUDIO_ARENA_SIZE_BYTES) {
            // Out of capacity bounds
            Logger::ERR(L"CRITICAL ERROR: Audio Memory Arena has overflowed");
            return nullptr; 
        }
        uint8_t* address = &mMemoryArena[mArenaOffset];
        mArenaOffset += size;
        return address;
    }
#pragma endregion
}