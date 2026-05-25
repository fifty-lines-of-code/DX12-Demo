#pragma once

#include "../IInputSystem.h"

#define NOMINMAX
#include <windows.h>
#include <xinput.h>

// Automatically link Microsoft's XInput library
#pragma comment(lib, "XInput.lib")

class XboxInputSystem : public IInputSystem {
public:
    // 'explicit' prevents the compiler from using this constructor to silently 
    // convert a raw DWORD (like 0) into an XboxInputSystem object behind our backs.
    explicit XboxInputSystem(DWORD userIndex = 0);
    virtual ~XboxInputSystem() override = default;

    // Core Frame Update
    virtual void Update() override;
    virtual bool IsConnected() const override;

    // Left Analog Stick Queries
    virtual float GetLeftStickX() const override;
    virtual float GetLeftStickY() const override;

    // Right Analog Stick Queries
    virtual float GetRightStickX() const override;
    virtual float GetRightStickY() const override;

    virtual GameButtonState GetButtonState(GameButton button) const override;

    virtual float GetLeftTrigger() const override;
    virtual float GetRightTrigger() const override;

private:
    DWORD mUserIndex;
    bool mIsConnected;
    XINPUT_STATE mCurrentState;
    XINPUT_STATE mPreviousState;
    float mSquareOfLeftJoystickDeadzone;
    float mSquareOfRightJoystickDeadzone;

    // Processed, deadzone-filtered axis coordinates
    float mLeftStickX;
    float mLeftStickY;
    float mRightStickX;
    float mRightStickY;

private:
    void UpdateStateAndIsConnected();
    void ProcessLeftJoystick();
    void ProcessRightJoystick();
    void ProcessTriggers();
    void ProcessXYForJoystick(int16_t rawX, int16_t rawY, float* stickX, float* stickY, float squareOfDeadzone);
    virtual uint32_t GetControllerMappingFor(GameButton button) const override;
};