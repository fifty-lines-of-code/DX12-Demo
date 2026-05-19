#pragma once

#include <cstdint>

// The values use specific hexadecimal bitmasks so that multiple buttons 
// can be packed into a single integer and queried simultaneously.
// Example combining two buttons into a single combo bitmask:
// uint32_t superMoveCombo = (uint32_t)GameButton::ActionSouth | (uint32_t)GameButton::ActionEast;
// If (currentState & superMoveCombo) == superMoveCombo (which evaluates to 0x0003),
// it means both buttons are being held down at the exact same millisecond.

enum class GameButton {
    // 1. Right-Hand Action Cluster
    ActionSouth = 0x0001, // Xbox A / PS Cross
    ActionEast = 0x0002, // Xbox B / PS Circle
    ActionWest = 0x0004, // Xbox X / PS Square
    ActionNorth = 0x0008, // Xbox Y / PS Triangle

    // 2. Index Finger Shoulders
    BumperLeft = 0x0010, // Xbox LT / PS L1 
    BumperRight = 0x0020, // Xbox RT / PS R1

    // 3. Stick Click Modifiers (Physical Analogs as buttons)
    StickClickLeft = 0x0040, // Xbox LS  / PS L3
    StickClickRight = 0x0080, // Xbox RS  / PS R3

    // 4. Directional Pad
    DPadUp = 0x0100, // Up
    DPadDown = 0x0200, // Down
    DPadLeft = 0x0400, // Left
    DPadRight = 0x0800, // Right

    // 5. System Administration Controls
    CenterRight = 0x1000, // Xbox Start / PS Options
    CenterLeft = 0x2000  // Xbox Back  / PS Share
};

enum class GameButtonState {
    Unpressed = 0,
    Pressed
};