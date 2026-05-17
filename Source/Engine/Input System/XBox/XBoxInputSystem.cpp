#include "XBoxInputSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

XBoxInputSystem::XBoxInputSystem(DWORD userIndex) :
    mUserIndex(userIndex),
    mIsConnected(false),
    mLeftStickX(0.f),
    mLeftStickY(0.f),
    mRightStickX(0.f),
    mRightStickY(0.f) {

    ZeroMemory(&mCurrentState, sizeof(XINPUT_STATE));

    // Calculate square of raw deadzones to prevent sqrt calls per frame
    float rawLeftDeadzone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    mSquareOfLeftJoystickDeadzone = rawLeftDeadzone * rawLeftDeadzone;

    float rawRightDeadzone = static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    mSquareOfRightJoystickDeadzone = rawRightDeadzone * rawRightDeadzone;
}

void XBoxInputSystem::Update() {
    UpdateStateAndIsConnected();
    ProcessLeftJoystick();
    ProcessRightJoystick();
    ProcessActionButtons();
}

bool XBoxInputSystem::IsConnected() const {
    return mIsConnected;
}

float XBoxInputSystem::GetLeftStickX() const {
    return mLeftStickX;
}

float XBoxInputSystem::GetLeftStickY() const {
    return mLeftStickY;
}

float XBoxInputSystem::GetRightStickX() const {
    return mRightStickX;
}

float XBoxInputSystem::GetRightStickY() const {
    return mRightStickY;
}

bool XBoxInputSystem::IsButtonDown(GameButtons button) const {
    // todo
    return false;
}

bool XBoxInputSystem::IsButtonPressed(GameButtons button) const {
    // todo
    return false;
}

bool XBoxInputSystem::IsButtonReleased(GameButtons button) const {
    // todo
    return false;
}

float XBoxInputSystem::GetLeftTrigger() const {
    // todp
    return 0.0f;
}

float XBoxInputSystem::GetRightTrigger() const {
    // todo
    return 0.0f;
}

void XBoxInputSystem::UpdateStateAndIsConnected() {
    ZeroMemory(&mCurrentState, sizeof(mCurrentState));
    DWORD result = XInputGetState(mUserIndex, &mCurrentState);
    mIsConnected = (result == ERROR_SUCCESS);
}

void XBoxInputSystem::ProcessLeftJoystick() {
    if (!mIsConnected) {
        mLeftStickX = 0.f;
        mLeftStickY = 0.f;
        return;
    }

    auto rawX = mCurrentState.Gamepad.sThumbLX;
    auto rawY = mCurrentState.Gamepad.sThumbLY;

    ProcessXYForJoystick(rawX, rawY, &mLeftStickX, &mLeftStickY, mSquareOfLeftJoystickDeadzone);
}

void XBoxInputSystem::ProcessRightJoystick() {
    if (!mIsConnected) {
        mRightStickX = 0.f;
        mRightStickY = 0.f;
        return;
    }

    auto rawX = mCurrentState.Gamepad.sThumbRX;
    auto rawY = mCurrentState.Gamepad.sThumbRY;

    ProcessXYForJoystick(rawX, rawY, &mRightStickX, &mRightStickY, mSquareOfRightJoystickDeadzone);
}

void XBoxInputSystem::ProcessActionButtons() {
    // todo
}

void XBoxInputSystem::ProcessTriggers() {
    //  todo
}

void XBoxInputSystem::ProcessXYForJoystick(int16_t rawX, int16_t rawY, float* stickX, float* stickY, float squaredDeadzone) {
    float floatX = static_cast<float>(rawX);
    float floatY = static_cast<float>(rawY);

    float squaredMagnitude = (floatX * floatX) + (floatY * floatY);

    if (squaredMagnitude < squaredDeadzone) {
        *stickX = 0.f;
        *stickY = 0.f;
    }
    else {
        // Automatically deduce the type directly from the hardware state struct
        using MicrosoftAxisType = decltype(mCurrentState.Gamepad.sThumbLX);
        constexpr float maxPossibleValue = static_cast<float>(std::numeric_limits<MicrosoftAxisType>::max());

        // Perform the scaling completely free of magic numbers
        float normalizedX = floatX / maxPossibleValue;
        float normalizedY = floatY / maxPossibleValue;

        *stickX = std::max(-1.0f, std::min(normalizedX, 1.0f));
        *stickY = std::max(-1.0f, std::min(normalizedY, 1.0f));
    }
}