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
    // 
    // Example of what it blocks:
    //   void ProcessInput(const XboxInputSystem& system);
    //   ProcessInput(0); // ERROR: Blocked from silently casting 0 into an object!
    //
    // Example of what it forces:
    //   XboxInputSystem mainController(0); // ALLOWED: Explicitly constructed by the coder.
    // ProcessInput(mainController)

	explicit XBoxInputSystem(DWORD userIndex = 0);
	virtual ~XBoxInputSystem() override = default;

    // Core Frame Update
    virtual void Update() override;
    virtual bool IsConnected() const override { return mIsConnected; }

    // Left Analog Stick Queries
    virtual float GetLeftStickX() const override { return mLeftStickX; }
    virtual float GetLeftStickY() const override { return mLeftStickY; }

    // Stubs for the rest of the interface to compile cleanly
    virtual bool IsButtonDown(GameButtons button) const override { return false; }
    virtual bool IsButtonPressed(GameButtons button) const override { return false; }
    virtual bool IsButtonReleased(GameButtons button) const override { return false; }
    virtual float GetRightStickX() const override { return 0.0f; }
    virtual float GetRightStickY() const override { return 0.0f; }
    virtual float GetLeftTrigger() const override { return 0.0f; }
    virtual float GetRightTrigger() const override { return 0.0f; }

private:
    DWORD mUserIndex;
    bool mIsConnected;
    XINPUT_STATE mCurrentState;
    float mSquareOfDeadzone = 0;

    // Processed, deadzone-filtered axis coordinates
    float mLeftStickX;
    float mLeftStickY;

    void UpdateStateAndIsConnected();
    void ProcessLeftJoystick();
};