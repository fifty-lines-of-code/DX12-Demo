#include "PlayerAnimator.h"

#include <cmath>
#include "../../../Math/MathHelper.h"

PlayerAnimator::PlayerAnimator() : 
	mCurrentRotation(0.f)
{}

PlayerAnimator::~PlayerAnimator() {}

void PlayerAnimator::UpdateVisualRotation(
	float deltaTime,
	const Engine::Vector3& movement,
	float rotationSpeed
) {
	// We have to send in x first and then z to convert from 
	// Math's RH rule to DX12's LH coordinate rule
	// Rotation in Math is CCW and DX12 is CW
	float targetRotation = std::atan2(movement.x, movement.z);
	float deltaRotation = targetRotation - mCurrentRotation;

	// we have to make sure we take the shortest rotation 
	// so rotate -90 instead of 270
	// to do that we subtract 2pi if delta is > pi
	// and add 2pi if delta is < -pi
	// since rotation values will accumulate, we do this over a loop

	while (deltaRotation > Engine::MathHelper::Pi) { 
		deltaRotation -= Engine::MathHelper::Two_Pi; 
	}

	while (deltaRotation < -Engine::MathHelper::Pi) { 
		deltaRotation += Engine::MathHelper::Two_Pi; 
	}

	if (std::abs(deltaRotation) < 0.01f) {
		mCurrentRotation = targetRotation;
		mIsRotationComplete = true;
		return;
	}

	// now we smoothly interpolate to the targetRotation
	mCurrentRotation += deltaRotation * rotationSpeed * deltaTime;
}

void PlayerAnimator::AnimateBackwardsDash(
	float deltaTime,
	float animationDuration
) {
	if (!mDashAnimationData.IsPerformingBackwardsDash) {
		// Frame 1: Setup the trajectory
		mDashAnimationData.IsPerformingBackwardsDash = true;
		mDashAnimationData.IsBackwardsDashAnimationComplete = false;
		mDashAnimationData.DashAnimationTimer = 0.f;
	}
	else {
		// update the timer
		mDashAnimationData.DashAnimationTimer += deltaTime;

		if (mDashAnimationData.DashAnimationTimer >= animationDuration) {
			ResetBackwardsDashAnimation();
		}
	}
}

bool PlayerAnimator::GetIsBackwardsDashAnimationComplete() const {
	return mDashAnimationData.IsBackwardsDashAnimationComplete;
}

void PlayerAnimator::ResetBackwardsDashAnimation() {
	mDashAnimationData.IsPerformingBackwardsDash = false;
	mDashAnimationData.IsBackwardsDashAnimationComplete = true;
	mDashAnimationData.DashAnimationTimer = 0.f;
}

float PlayerAnimator::GetRotation() const { return mCurrentRotation; }

bool PlayerAnimator::GetIsRotationComplete() const { return mIsRotationComplete; }

void PlayerAnimator::SetIsRotationComplete(bool value) { mIsRotationComplete = value; }