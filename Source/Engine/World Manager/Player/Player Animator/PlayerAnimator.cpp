#include "PlayerAnimator.h"

#include <cmath>
#include "../../../Math/MathHelper.h"

PlayerAnimator::PlayerAnimator() : 
	mCurrentRotation(DirectX::XMConvertToRadians(180))
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

	// Our player mesh faces -z natively (see MeshGenerator),
	// so we add PI to offset it so the mesh faces forward (+Z) when moving North.
	float targetRotation = std::atan2(movement.x, movement.z) + Engine::MathHelper::Pi;
	float deltaRotation = targetRotation - mCurrentRotation;

	// Shortest path optimization:
	// Subtract 2pi if delta is > pi, and add 2pi if delta is < -pi
	while (deltaRotation > Engine::MathHelper::Pi) {
		deltaRotation -= Engine::MathHelper::Two_Pi;
	}

	while (deltaRotation < -Engine::MathHelper::Pi) {
		deltaRotation += Engine::MathHelper::Two_Pi;
	}

	// Completion check
	if (std::abs(deltaRotation) < 0.01f) {
		mCurrentRotation = targetRotation;
		mIsRotationComplete = true;
		return;
	}

	// Smoothly interpolate to the targetRotation
	mCurrentRotation += deltaRotation * rotationSpeed * deltaTime;

	// Bounding Safety Wrap:
	// Keeps mCurrentRotation strictly locked inside the [-PI, PI] range.
	// This prevents the shortest-path while-loops above from accumulating multi-loop overhead 
	// when the player runs in circles continuously.
	while (mCurrentRotation > Engine::MathHelper::Pi) {
		mCurrentRotation -= Engine::MathHelper::Two_Pi;
	}
	while (mCurrentRotation < -Engine::MathHelper::Pi) {
		mCurrentRotation += Engine::MathHelper::Two_Pi;
	}
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