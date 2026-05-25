#pragma once

#include "../../../Engine/Math/EngineMath.h"

struct BasisVectors;

class PlayerAnimator {
public:
	PlayerAnimator();
	~PlayerAnimator();

	bool MoveAndRotatePlayer(
		float deltaTime,
		const Engine::Vector3* const movement,
		Engine::Vector3* const center, 
		float* currentRotation,
		float walkingRunningSpeed,
		float rotationSpeed
	);
	void RotatePlayer(
		float deltaTime,
		const Engine::Vector3* const movement,
		float rotationSpeed,
		float* currentRotation
	);
	bool PerformBackwardsDash(
		float deltaTime, 
		Engine::Vector3* const center, 
		const Engine::Vector3* const forward,
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