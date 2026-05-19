#pragma once

#include "../IInputSystem.h"

#define NOMINMAX
#include <windows.h>
#include <xinput.h>

// Automatically link Microsoft's XInput library
#pragma comment(lib, "XInput.lib")

class XBoxInputSystem : public IInputSystem {
public:
    // 'explicit' prevents the compiler from using this constructor to silently 
    // convert a raw DWORD (like 0) into an XboxInputSystem object behind our backs.
    explicit XBoxInputSystem(DWORD userIndex = 0);
    virtual ~XBoxInputSystem() override = default;

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

    // Stubs for the rest of the interface to compile cleanly
    virtual bool IsButtonDown(GameButton button) const override;
    virtual bool IsButtonPressed(GameButton button) const override;
    virtual bool IsButtonReleased(GameButton button) const override;

    virtual float GetLeftTrigger() const override;
    virtual float GetRightTrigger() const override;

    virtual uint32_t GetControllerMappingFor(GameButton button) const override;

private:
    DWORD mUserIndex;
    bool mIsConnected;
    XINPUT_STATE mCurrentState;
    float mSquareOfLeftJoystickDeadzone;
    float mSquareOfRightJoystickDeadzone;

    // Processed, deadzone-filtered axis coordinates
    float mLeftStickX;
    float mLeftStickY;
    float mRightStickX;
    float mRightStickY;

    bool mIsBPressed;

    void UpdateStateAndIsConnected();
    void ProcessLeftJoystick();
    void ProcessRightJoystick();
    void ProcessActionButtons();
    void ProcessTriggers();
    void ProcessXYForJoystick(int16_t rawX, int16_t rawY, float* stickX, float* stickY, float squareOfDeadzone);
};