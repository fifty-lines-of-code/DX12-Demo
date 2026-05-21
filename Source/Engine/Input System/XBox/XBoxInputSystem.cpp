#include "XboxInputSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

XboxInputSystem::XboxInputSystem(DWORD userIndex) :
    mUserIndex(userIndex),
    mIsConnected(false),
    mLeftStickX(0.f),
    mLeftStickY(0.f),
    mRightStickX(0.f),
    mRightStickY(0.f),
    mIsBPressed(false) {

    ZeroMemory(&mCurrentState, sizeof(XINPUT_STATE));

    // Calculate squares of raw deadzones to prevent sqrt calls per frame
    float rawLeftDeadzone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    mSquareOfLeftJoystickDeadzone = rawLeftDeadzone * rawLeftDeadzone;

    float rawRightDeadzone = static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    mSquareOfRightJoystickDeadzone = rawRightDeadzone * rawRightDeadzone;
}

void XboxInputSystem::Update() {
    mPreviousState = mCurrentState;
    UpdateStateAndIsConnected();
    ProcessLeftJoystick();
    ProcessRightJoystick();
}

bool XboxInputSystem::IsConnected() const {
    return mIsConnected;
}

float XboxInputSystem::GetLeftStickX() const {
    return mLeftStickX;
}

float XboxInputSystem::GetLeftStickY() const {
    return mLeftStickY;
}

float XboxInputSystem::GetRightStickX() const {
    return mRightStickX;
}

float XboxInputSystem::GetRightStickY() const {
    return mRightStickY;
}

GameButtonState XboxInputSystem::GetButtonState(GameButton button) const {
    uint32_t mapping = GetControllerMappingFor(button);
    uint32_t buttonStateRawThisFrame = mCurrentState.Gamepad.wButtons & mapping;
    uint32_t buttonStateRawPreviousFrame = mPreviousState.Gamepad.wButtons & mapping;

    if (buttonStateRawThisFrame == 0 && buttonStateRawPreviousFrame == 0) {
        return GameButtonState::Unpressed;
    }

    if (buttonStateRawThisFrame != 0 && buttonStateRawPreviousFrame == 0) {
        return GameButtonState::Just_Pressed;
    }

    if (buttonStateRawThisFrame != 0 && buttonStateRawPreviousFrame != 0) {
        return GameButtonState::Held;
    }

    return GameButtonState::Just_Released;
}

float XboxInputSystem::GetLeftTrigger() const {
    // todp
    return 0.0f;
}

float XboxInputSystem::GetRightTrigger() const {
    // todo
    return 0.0f;
}

void XboxInputSystem::UpdateStateAndIsConnected() {
    ZeroMemory(&mCurrentState, sizeof(mCurrentState));
    DWORD result = XInputGetState(mUserIndex, &mCurrentState);
    mIsConnected = (result == ERROR_SUCCESS);
}

void XboxInputSystem::ProcessLeftJoystick() {
    if (!mIsConnected) {
        mLeftStickX = 0.f;
        mLeftStickY = 0.f;
        return;
    }

    auto rawX = mCurrentState.Gamepad.sThumbLX;
    auto rawY = mCurrentState.Gamepad.sThumbLY;

    ProcessXYForJoystick(rawX, rawY, &mLeftStickX, &mLeftStickY, mSquareOfLeftJoystickDeadzone);
}

void XboxInputSystem::ProcessRightJoystick() {
    if (!mIsConnected) {
        mRightStickX = 0.f;
        mRightStickY = 0.f;
        return;
    }

    auto rawX = mCurrentState.Gamepad.sThumbRX;
    auto rawY = mCurrentState.Gamepad.sThumbRY;

    ProcessXYForJoystick(rawX, rawY, &mRightStickX, &mRightStickY, mSquareOfRightJoystickDeadzone);
}

void XboxInputSystem::ProcessTriggers() {
    //  todo
}

void XboxInputSystem::ProcessXYForJoystick(int16_t rawX, int16_t rawY, float* stickX, float* stickY, float squaredDeadzone) {
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

uint32_t XboxInputSystem::GetControllerMappingFor(GameButton button) const {
    switch (button) {
    case GameButton::ActionEast:
        return XINPUT_GAMEPAD_B;
    }

    // todo:
    return 0;
}