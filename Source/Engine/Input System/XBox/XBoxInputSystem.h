#pragma once

#include "../IInputSystem.h"
#include <atomic>
#include <thread>

#define NOMINMAX
#include <windows.h>
#include <xinput.h>

// Automatically link Microsoft's XInput library
#pragma comment(lib, "XInput.lib")

struct XboxInputState {
    XINPUT_STATE CurrentState;
    XINPUT_STATE PreviousState;
    bool mIsConnected = false;
    // Processed, deadzone-filtered axis coordinates
    float mLeftStickX = 0.f;
    float mLeftStickY = 0.f;
    float mRightStickX = 0.f;
    float mRightStickY = 0.f;
};

class XboxInputSystem : public IInputSystem {
public:
    // 'explicit' prevents the compiler from using this constructor to silently 
    // convert a raw DWORD (like 0) into an XboxInputSystem object behind our backs.
    explicit XboxInputSystem(DWORD userIndex = 0);
    virtual ~XboxInputSystem() override;

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
    static const uint16_t Number_Of_Buffers = 3;
    XboxInputState mInputStatePool[XboxInputSystem::Number_Of_Buffers];
    std::atomic<uint16_t> mStagingIndex;
    uint16_t mBackgroundIndex;
    uint16_t mRenderIndex;

    DWORD mUserIndex;
    float mSquareOfLeftJoystickDeadzone;
    float mSquareOfRightJoystickDeadzone;

    std::thread mBackgroundUpdateThread;
    std::atomic<bool> mIsRunning;

private:
    void BackgroundUpdateThreadTick();
    void UpdateStateAndIsConnected();
    void ProcessLeftJoystick();
    void ProcessRightJoystick();
    void ProcessTriggers();
    void ProcessXYForJoystick(int16_t rawX, int16_t rawY, float* stickX, float* stickY, float squareOfDeadzone);
    virtual uint32_t GetControllerMappingFor(GameButton button) const override;
};