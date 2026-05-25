#include "XboxInputSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

XboxInputSystem::XboxInputSystem(DWORD userIndex) :
    mUserIndex(userIndex),
    mIsRunning(true),
    mStagingIndex(0),
    mBackgroundIndex(1),
    mRenderIndex(2)
{
    for (int i = 0; i < XboxInputSystem::Number_Of_Buffers; ++i) {
        ZeroMemory(&mInputStatePool[i].CurrentState, sizeof(XINPUT_STATE));
        ZeroMemory(&mInputStatePool[i].PreviousState, sizeof(XINPUT_STATE));
    }

    // Calculate squares of raw deadzones to prevent sqrt calls per frame
    float rawLeftDeadzone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    mSquareOfLeftJoystickDeadzone = rawLeftDeadzone * rawLeftDeadzone;

    float rawRightDeadzone = static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    mSquareOfRightJoystickDeadzone = rawRightDeadzone * rawRightDeadzone;

    // spin up the worker thread
    mBackgroundUpdateThread = std::thread(&XboxInputSystem::BackgroundUpdateThreadTick, this);
}

XboxInputSystem::~XboxInputSystem() {
    mIsRunning = false;
    if (mBackgroundUpdateThread.joinable()) {
        mBackgroundUpdateThread.join();
    }
}

void XboxInputSystem::Update() {
    // first copy the "current" data into a temp var
    XINPUT_STATE currentStateFromLastFrame = mInputStatePool[mRenderIndex].CurrentState;
    // update the render index
    mRenderIndex = mStagingIndex.exchange(mRenderIndex);
    // restore the "current" data from last frame into previous state
    mInputStatePool[mRenderIndex].PreviousState = currentStateFromLastFrame;
}

bool XboxInputSystem::IsConnected() const {
    return mInputStatePool[mRenderIndex].mIsConnected;
}

float XboxInputSystem::GetLeftStickX() const {
    return mInputStatePool[mRenderIndex].mLeftStickX;
}

float XboxInputSystem::GetLeftStickY() const {
    return mInputStatePool[mRenderIndex].mLeftStickY;
}

float XboxInputSystem::GetRightStickX() const {
    return mInputStatePool[mRenderIndex].mRightStickX;
}

float XboxInputSystem::GetRightStickY() const {
    return mInputStatePool[mRenderIndex].mRightStickY;
}

GameButtonState XboxInputSystem::GetButtonState(GameButton button) const {
    uint32_t mapping = GetControllerMappingFor(button);
    XboxInputState inputState = mInputStatePool[mRenderIndex];

    uint32_t buttonStateRawThisFrame = inputState.CurrentState.Gamepad.wButtons & mapping;
    uint32_t buttonStateRawPreviousFrame = inputState.PreviousState.Gamepad.wButtons & mapping;

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
    // todo
    return 0.0f;
}

float XboxInputSystem::GetRightTrigger() const {
    // todo
    return 0.0f;
}

void XboxInputSystem::BackgroundUpdateThreadTick() {
    while (mIsRunning) {
        // sleep for 8 milliseconds 
        std::this_thread::sleep_for(std::chrono::milliseconds(8));

        // update the data
        UpdateStateAndIsConnected();
        ProcessLeftJoystick();
        ProcessRightJoystick();

        // exhange the background and staging buffers
        mBackgroundIndex = mStagingIndex.exchange(mBackgroundIndex);
    }
}

void XboxInputSystem::UpdateStateAndIsConnected() {
    ZeroMemory(
        &mInputStatePool[mBackgroundIndex].CurrentState, 
        sizeof(XboxInputState::CurrentState)
    );
    DWORD result = XInputGetState(
        mUserIndex,
        &mInputStatePool[mBackgroundIndex].CurrentState
    );
    mInputStatePool[mBackgroundIndex].mIsConnected = (result == ERROR_SUCCESS);
}

void XboxInputSystem::ProcessLeftJoystick() {
    if (!mInputStatePool[mBackgroundIndex].mIsConnected) {
        mInputStatePool[mBackgroundIndex].mLeftStickX = 0.f;
        mInputStatePool[mBackgroundIndex].mLeftStickY = 0.f;
        return;
    }

    auto gamepad = mInputStatePool[mBackgroundIndex].CurrentState.Gamepad;
    auto rawX = gamepad.sThumbLX;
    auto rawY = gamepad.sThumbLY;

    ProcessXYForJoystick(
        rawX, 
        rawY,
        &mInputStatePool[mBackgroundIndex].mLeftStickX,
        &mInputStatePool[mBackgroundIndex].mLeftStickY,
        mSquareOfLeftJoystickDeadzone
    );
}

void XboxInputSystem::ProcessRightJoystick() {
    if (!mInputStatePool[mBackgroundIndex].mIsConnected) {
        mInputStatePool[mBackgroundIndex].mRightStickX = 0.f;
        mInputStatePool[mBackgroundIndex].mRightStickY = 0.f;
        return;
    }

    auto gamepad = mInputStatePool[mBackgroundIndex].CurrentState.Gamepad;
    auto rawX = gamepad.sThumbRX;
    auto rawY = gamepad.sThumbRY;

    ProcessXYForJoystick(
        rawX,
        rawY,
        &mInputStatePool[mBackgroundIndex].mRightStickX,
        &mInputStatePool[mBackgroundIndex].mRightStickY,
        mSquareOfRightJoystickDeadzone
    );
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
        using MicrosoftAxisType = decltype(mInputStatePool[mBackgroundIndex].CurrentState.Gamepad.sThumbLX);
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