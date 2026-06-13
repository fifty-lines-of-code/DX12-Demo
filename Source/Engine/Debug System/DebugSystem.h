#pragma once

#include <array>
#include "DebugLimits.h"
#include "DebugSystemDataStructures.h"
#include "../Math/EngineMath.h"
#include <string>

namespace Engine::DebugSystem {

    using TextLogArray = std::array<DebugTextSlot, DebugLimits::MAX_CHARACTERS>;
    using TextVerticesArray = std::array<DebugSystemPerCharacterData, DebugLimits::MAX_CHARACTERS>;

    class DebugSystem {
    public:
        // inspiration for private Constructor and Getter
        // my experience with Objective-C
        static DebugSystem& GetInstance() {
            static DebugSystem instance;
            return instance;
        }

        DebugSystem(const DebugSystem&) = delete;
        DebugSystem& operator=(const DebugSystem&) = delete;

        void Initialize(uint32_t windowWidth, uint32_t windowHeight);
        void LogText(const std::string& text, const Vector4& color = {1.f, 0.f, 0.f, 1.f});

        void CalculateFramePositions();
        void ClearFrameCache();
        void UpdateWindowDimensions(uint32_t width, uint32_t height);

        bool GetIsDirty() const noexcept;
        void SetIsDirty(bool isDirty) noexcept;
        const TextVerticesArray& GetVertices() const noexcept;
        uint32_t GetTotalNumberOfCharacersToDraw() const noexcept;
        void GetPerPassCbData(DebugSystemPerPassCbData& data) const noexcept;

    private:
        TextVerticesArray mVertices;
        TextLogArray mTextPool;
        FontAtlasDesc mFontAtlasDesc;
        uint32_t mWindowWidth;
        uint32_t mWindowHeight;
        uint32_t mActiveLogCount;
        uint32_t mTotalNumberOfCharactersDrawn;   
        float mUVCellWidth;
        bool mIsDirty;

    private:
        DebugSystem();
        ~DebugSystem();

        void SetupFontAtlasForDebugFont();
    };
}