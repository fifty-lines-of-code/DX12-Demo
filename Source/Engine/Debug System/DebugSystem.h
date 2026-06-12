#pragma once

#include <array>
#include "DebugLimits.h"
#include "DebugSystemDataStructures.h"
#include "../Math/EngineMath.h"

namespace Engine::DebugSystem {

    using TextLogArray = std::array<DebugTextInstance, DebugLimits::MaxLogInstances>;
    using TextVeticesArray = std::array<TextVertex, DebugLimits::MaxVertices>;

    class DebugSystem {
    public:
        static DebugSystem& GetInstance() {
            static DebugSystem instance;
            return instance;
        }

        DebugSystem(const DebugSystem&) = delete;
        DebugSystem& operator=(const DebugSystem&) = delete;

        void Initialize(const FontAtlasDesc& fontDesc, uint32_t windowWidth, uint32_t windowHeight);
        void LogText(const std::string& text, const Vector4& color);

        void CompileFrameGeometry();
        void ClearFrameCache();

        const TextVeticesArray& GetVertices() const noexcept;
        const std::array<uint16_t, DebugLimits::MaxIndices>& GetIndices() const noexcept;
        uint32_t GetActiveVertexCount() const noexcept;
        uint32_t GetActiveIndexCount()  const noexcept;

    private:
        TextVeticesArray                                mCompiledVertices;
        std::array<uint16_t, DebugLimits::MaxIndices>   mCompiledIndices;
        TextLogArray                                    mTextInstancesPool;
        FontAtlasDesc                                   mFontDesc;
        uint32_t                                        mWindowWidth;
        uint32_t                                        mWindowHeight;
        uint32_t                                        mActiveLogCount;
        uint32_t                                        mActiveVertexCount;
        uint32_t                                        mActiveIndexCount;

        // Layout Configuration Constants
 
        // Safe space: 20 pixels from the left edge
        const float                                     mMarginX = 20.f;
        // Safe space: 20 pixels down from the top edge
        const float                                     mMarginY = 20.f;       
        // Pixels of padding to leave between lines of text
        const float                                     mLineSpacing = 4.f;
        // Runtime Tracking Counter - Tracks the current Y offset for the next log entry
        float                                           mNextLineY = 0.f;         

    private:
        DebugSystem();
        ~DebugSystem();
    };

}