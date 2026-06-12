#pragma once

#include <cstdint>

namespace DebugLimits {
    constexpr uint32_t MaxLogInstances = 128;      // Maximum lines of text per frame
    constexpr uint32_t MaxCharacters = 2048;     // Maximum total characters allowed on screen
    constexpr uint32_t MaxVertices = MaxCharacters * 4; // 4 Vertices per character quad
    constexpr uint32_t MaxIndices = MaxCharacters * 6; // 6 Indices per character quad

    // --- ASCII Character Set Mapping Bound Limits ---
    constexpr char FirstPrintableChar = 32;   // Space ' '
    constexpr char LastPrintableChar = 126;  // Tilde '~'
}