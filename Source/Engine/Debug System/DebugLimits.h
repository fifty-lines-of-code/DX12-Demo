#pragma once

#include <cstdint>

namespace Engine::DebugSystem::DebugLimits {
    // Maximum total characters allowed on screen
    constexpr uint32_t MAX_CHARACTERS = 2048;
    constexpr uint32_t MARGIN = 10;
    constexpr uint32_t LINE_SPACING = 2;
    constexpr uint32_t QUAD_WIDTH = 30;
    constexpr uint32_t QUAD_HEIGHT = 30;

    // --- ASCII Character Set Mapping Bound Limits ---
    constexpr char FIRST_PRINTABLE_CHAR = 32;   // Space ' '
    constexpr char LAST_PRINTABLE_CHAR = 126;  // Tilde '~'
}