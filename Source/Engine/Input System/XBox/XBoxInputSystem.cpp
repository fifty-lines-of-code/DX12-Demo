#include "XBoxInputSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

XBoxInputSystem::XBoxInputSystem(DWORD userIndex) : 
	mUserIndex(userIndex), mIsConnected(false), mLeftStickX(0.f), mLeftStickY(0.f) {
	ZeroMemory(&mCurrentState, sizeof(XINPUT_STATE));

	//Calculate square of raw deadzone to prevent sqrt call per frame
	float rawDeadzone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	mSquareOfDeadzone = rawDeadzone * rawDeadzone;
}

void XBoxInputSystem::Update() {
	UpdateStateAndIsConnected();
	ProcessLeftJoystick();
}

void XBoxInputSystem::UpdateStateAndIsConnected() {
	ZeroMemory(&mCurrentState, sizeof(mCurrentState));
	DWORD result = XInputGetState(mUserIndex, &mCurrentState);

	mIsConnected = result == ERROR_SUCCESS;
}

void XBoxInputSystem::ProcessLeftJoystick() {
	if (!mIsConnected) {
		mLeftStickX = 0.f;
		mLeftStickY = 0.f;
		return;
	}

	// 1. Grab the raw values
	auto rawX = mCurrentState.Gamepad.sThumbLX;
	auto rawY = mCurrentState.Gamepad.sThumbLY;

	// 2. calculate squre of magnitude of the vector (x + y)
	float squaredMagnitude = (rawX * rawX) + (rawY * rawY);

	// 3. Check if stick is inside or outside the deadzone
	if (squaredMagnitude < mSquareOfDeadzone) {
		mLeftStickX = 0.f;
		mLeftStickY = 0.f;
	} else {
		// stick is outside the deadzone, normalize rawX and Y
		float magnitude = std::sqrt(squaredMagnitude);

		using MicrosoftAxisType = decltype(mCurrentState.Gamepad.sThumbLX);
		constexpr float maxPossibleValue = static_cast<float>(std::numeric_limits<MicrosoftAxisType>::max());

		mLeftStickX = std::max(
			-1.0f, 
			std::min(static_cast<float>(rawX) / maxPossibleValue, 1.0f)
		);
		mLeftStickY = std::max(
			-1.0f,
			std::min(static_cast<float>(rawY) / maxPossibleValue, 1.0f)
		);
	}
}