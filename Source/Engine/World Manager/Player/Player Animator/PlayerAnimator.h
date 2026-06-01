#pragma once

#include "../../../../Engine/Math/EngineMath.h"

struct DashAnimationData {
	Engine::Vector3 DashStartPosition = Engine::Vector3(0, 0, 0);
	Engine::Vector3 DashTargetPosition = Engine::Vector3(0, 0, 0);
	Engine::Vector3 DashDirection = Engine::Vector3(0, 0, 0);
	float DashAnimationTimer = 0.5f;
	float IsPerformingBackwardsDash = false;
	float IsBackwardsDashAnimationComplete = true;
};

class PlayerAnimator {
public:
	PlayerAnimator();
	~PlayerAnimator();

	void UpdateVisualRotation(
		float deltaTime,
		const Engine::Vector3& movement,
		float rotationSpeed
	);
	void AnimateBackwardsDash(
		float deltaTime,
		float animationDuration
	);
	bool GetIsBackwardsDashAnimationComplete() const;
	void ResetBackwardsDashAnimation();
	float GetRotation() const;
	bool GetIsRotationComplete() const;
	void SetIsRotationComplete(bool value);

private:
	// dash animation data
	DashAnimationData mDashAnimationData;
	// Rotation Data
	float mCurrentRotation;
	bool mIsRotationComplete = false;

private:
};