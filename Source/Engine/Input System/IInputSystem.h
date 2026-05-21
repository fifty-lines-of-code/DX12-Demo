#pragma once

#include "GameButton.h"

class IInputSystem {
public:
	virtual ~IInputSystem() = default;

	virtual void Update() = 0;
    virtual void ProcessLeftJoystick() = 0;
    virtual void ProcessRightJoystick() = 0;
    virtual void ProcessTriggers() = 0;

    virtual bool IsConnected() const = 0;
    virtual GameButtonState GetButtonState(GameButton button) const = 0;

    virtual float GetLeftStickX() const = 0;
    virtual float GetLeftStickY() const = 0;
    virtual float GetRightStickX() const = 0;
    virtual float GetRightStickY() const = 0;
    virtual float GetLeftTrigger() const = 0;
    virtual float GetRightTrigger() const = 0;

private:
    virtual uint32_t GetControllerMappingFor(GameButton button) const = 0;
};