#pragma once

#include "AudioBufferAsset.h"
#include <cstdint>
#include <fstream>
#include "../../../Helper/Logger.h"

namespace Engine::EngineAudio {

    // Force the compiler to pack this struct tightly with 0 byte padding gaps
#pragma pack(push, 1)
    struct RigidWavHeader {
        char     riffId[4];      // Must be "RIFF"
        uint32_t fileSize;       // Total file size minus 8 bytes
        char     waveId[4];      // Must be "WAVE"
        char     fmtId[4];       // Must be "fmt "
        uint32_t fmtSize;        // Subchunk size (usually 16 for PCM)
        uint16_t audioFormat;    // Audio format (1 = PCM uncompressed)
        uint16_t numChannels;    // Mono = 1, Stereo = 2
        uint32_t sampleRate;     // e.g., 44100, 48000
        uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample/8
        uint16_t blockAlign;     // numChannels * bitsPerSample/8
        uint16_t bitsPerSample;  // 8, 16, 24, or 32 bits
        char     dataId[4];      // Must be "data"
        uint32_t dataSize;       // Size of the raw PCM payload
    };
#pragma pack(pop)

    class AudioManagerHelper {
    public:
        static bool LoadWavFileRaw(
            const std::wstring& path, 
            AudioBufferAsset& outAsset
        ) {
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) return false;

            // 1. Validate the mandatory RIFF/WAVE container format
            char mainHeader[12];
            if (!file.read(mainHeader, 12)) return false;

            std::string_view riffId(mainHeader, 4);
            std::string_view waveId(mainHeader + 8, 4);

            if (riffId != "RIFF" || waveId != "WAVE") {
                Logger::ERR((L"Engine Rule Violation: File is not a valid WAV container -> " + path).c_str());
                return false;
            }

            bool foundFmt = false;
            bool foundData = false;
            char chunkId[4];
            uint32_t chunkSize = 0;

            // 2. Fast-scan chunks to locate 'fmt ' and 'data' blocks safely
            while (file.read(chunkId, 4) && file.read(reinterpret_cast<char*>(&chunkSize), 4)) {
                std::string_view currentChunk(chunkId, 4);

                if (currentChunk == "fmt ") {
                    // Read standard format data
                    uint32_t bytesToRead = (chunkSize > sizeof(WAVEFORMATEX)) ? sizeof(WAVEFORMATEX) : chunkSize;
                    file.read(reinterpret_cast<char*>(&outAsset.waveFormat), bytesToRead);

                    // Skip any extra extended format properties if present
                    if (chunkSize > bytesToRead) {
                        file.seekg(chunkSize - bytesToRead, std::ios::cur);
                    }
                    outAsset.waveFormat.cbSize = 0;
                    foundFmt = true;
                }
                else if (currentChunk == "data") {
                    // Extract the raw PCM byte payload size
                    outAsset.dataSize = chunkSize;
                    outAsset.rawPCMBytes = new uint8_t[chunkSize];

                    file.read(reinterpret_cast<char*>(outAsset.rawPCMBytes), chunkSize);
                    foundData = true;
                    break; // Data found, safe to stop parsing
                }
                else {
                    // Instantly skip over non-audio chunks (metadata, JUNK tags, etc.)
                    // RIFF specs require aligning odd-sized chunks by adding 1 padding byte
                    uint32_t alignmentPadding = chunkSize % 2;
                    file.seekg(chunkSize + alignmentPadding, std::ios::cur);
                }
            }

            file.close();

            // 3. Strict Verification: Both necessary segments must be present
            if (!foundFmt || !foundData || outAsset.rawPCMBytes == nullptr) {
                Logger::ERR((L"Engine Rule Violation: Missing critical PCM structures inside WAV -> " + path).c_str());

                if (outAsset.rawPCMBytes) {
                    delete[] outAsset.rawPCMBytes;
                    outAsset.rawPCMBytes = nullptr;
                }
                return false;
            }

            // Ensure it's uncompressed standard PCM data (Format Tag 1)
            if (outAsset.waveFormat.wFormatTag != 1) {
                Logger::ERR((L"Engine Rule Violation: Only uncompressed PCM WAV files are supported -> " + path).c_str());
                delete[] outAsset.rawPCMBytes;
                outAsset.rawPCMBytes = nullptr;
                return false;
            }

            return true;
        }
    };
}