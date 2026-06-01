#pragma once

#include "../../../../Engine/Math/EngineMath.h"

struct BackwardsDashAnimationData {
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
	BackwardsDashAnimationData mDashAnimationData;
	// Rotation Data
	float mCurrentRotation;
	bool mIsRotationComplete = false;

private:
};