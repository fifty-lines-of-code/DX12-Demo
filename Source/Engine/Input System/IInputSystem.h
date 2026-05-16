#pragma once

#include "GameButtons.h"

class IInputSystem {
public:
	virtual ~IInputSystem() = default;

	virtual bool IsConnected() const = 0;
	virtual void Update() = 0;

    virtual bool IsButtonDown(GameButtons button) const = 0;
    virtual bool IsButtonPressed(GameButtons button) const = 0;
    virtual bool IsButtonReleased(GameButtons button) const = 0;

    virtual float GetLeftStickX() const = 0;
    virtual float GetLeftStickY() const = 0;
    virtual float GetRightStickX() const = 0;
    virtual float GetRightStickY() const = 0;
    virtual float GetLeftTrigger() const = 0;
    virtual float GetRightTrigger() const = 0;
};