#pragma once

#include "../../../../Engine/Math/EngineMath.h"

class PlayerAnimator {
public:
	PlayerAnimator();
	~PlayerAnimator();

	bool MoveAndRotatePlayer(
		float deltaTime,
		const Engine::Vector3& movement,
		Engine::Vector3& center, 
		float* currentRotation,
		float walkingRunningSpeed,
		float rotationSpeed
	);
	void RotatePlayer(
		float deltaTime,
		const Engine::Vector3& movement,
		float rotationSpeed,
		float* currentRotation
	);
	bool PerformBackwardsDash(
		float deltaTime, 
		Engine::Vector3& center, 
		const Engine::Vector3& forward,
		float backwardsDashDistance,
		float animationDuration
	);
	bool GetIsBackwardsDashAnimationComplete() const;
	bool GetIsRotationComplete() const;
	void SetIsRotationComplete(bool value);

private:
	// dash animation data
	Engine::Vector3 mDashStartPosition = Engine::Vector3(0, 0, 0);
	Engine::Vector3 mDashTargetPosition = Engine::Vector3(0, 0, 0);
	float mDashAnimationTimer = 0.5f;
	float mIsPerformingBackwardsDash = false;
	float mIsBackwardsDashAnimationComplete = true;
	Engine::Vector3 mDashDirection = Engine::Vector3(0, 0, 0);
	bool mIsRotationComplete = false;

private:
};