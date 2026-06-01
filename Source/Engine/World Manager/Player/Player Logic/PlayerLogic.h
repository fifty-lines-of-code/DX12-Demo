#pragma once

#include <DirectXMath.h>
#include "../../../../Engine/Input System/IInputSystem.h"

enum class PlayerState {
	Idle,
	Walking,
	Running,
	PendingActionEast,
	BackwardsDashing
};

class PlayerLogic {
public:
	PlayerLogic();
	~PlayerLogic();

	PlayerState GetPlayerState() const;
	void SetPlayerState(PlayerState state);
	void Update(float deltaTime, const IInputSystem* const inputSystem);

	float GetWalkingRunningSpeed();

	// traversal and rotation speeds, achieved by trial and error
	const float mPlayerSlowWalkingSpeed = 0.2f;
	const float mPlayerNormalWalkingSpeed = .375f;
	const float mPlayerRunningSpeed = .65f;
	const float mRotationSpeed = 9.21f;
	// backwards dash animation data
	const float mBackwardsDashAnimationDuration = .85f;
	// speed = distance / time
	// dash distance is 1
	// so (1 / .85)
	const float mBackwardsDashVelocity = 1.18f; 

private:
	// total duration between holding action east (b or circle) 
	// and going up on left joypad to run instead of backwards dash
	// assuming player is idle
	const float mTotalDurationPlayerHasToRunAfterPressingActionEast = 0.055f;
	// duration and distance of backwards dash

	// -1 means wasn't pressed previous frame
	// 0 means was pressed previous frame
	// > 0 means totalDuration it's been pressed
	float mTotalTimeActionEastWasHeldDown = -1;

	// current state
	PlayerState mState = PlayerState::Idle;

private:
	void CalculateCurrentState(float deltaTime, const IInputSystem* const inputSystem);
};