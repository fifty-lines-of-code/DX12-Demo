#include "PlayerAnimator.h"

#include "../../../Engine/Camera/Camera.h"
#include <cmath>
#include "../../../Helper/Logger.h"

PlayerAnimator::PlayerAnimator() {}

PlayerAnimator::~PlayerAnimator() {}

bool PlayerAnimator::MoveAndRotatePlayer(
	float deltaTime,
	const Engine::Vector3* const movement,
	Engine::Vector3* const center,
	float* currentRotation,
	float walkingRunningSpeed,
	float rotationSpeed
) {
	// update center
	center->x += movement->x * walkingRunningSpeed * deltaTime;
	// todo: hardcoded for now. will take y into consideration when ready
	center->y = 0.6f;
	center->z += movement->z * walkingRunningSpeed * deltaTime;

	// update rotation
	RotatePlayer(deltaTime, movement, rotationSpeed, currentRotation);

	return true;
}

void PlayerAnimator::RotatePlayer(
	float deltaTime,
	const Engine::Vector3* const movement,
	float rotationSpeed,
	float* currentRotation
) {
	// We have to send in x first and then z to convert from 
	// Math's RH rule to DX12's LH coordinate rule
	// Rotation in Math is CCW and DX12 is CW
	float targetRotation = std::atan2(movement->x, movement->z);
	float deltaRotation = targetRotation - *currentRotation;

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

	Logger::PRINT(
		L"Delta Rotation is: " +
		std::to_wstring(deltaRotation) +
		L"\n"
	);

	if (std::abs(deltaRotation) < 0.01f) {
		*currentRotation = targetRotation;
		mIsRotationComplete = true;
		return;
	}

	// now we smoothly interpolate to the targetRotation
	*currentRotation += deltaRotation * rotationSpeed * deltaTime;
}

bool PlayerAnimator::PerformBackwardsDash(
	float deltaTime, 
	Engine::Vector3* const center,
	const Engine::Vector3* const forward,
	float backwardsDashVelocity,
	float animationDuration
) {
	if (!mIsPerformingBackwardsDash) {
		// Frame 1: Setup the trajectory
		mIsPerformingBackwardsDash = true;
		mIsBackwardsDashAnimationComplete = false;
		mDashAnimationTimer = 0.f;

		// Calculate and store the direction (flipped forward vector)
		// We flatten the Y axis to keep the dash strictly in the xz plane for now
		mDashDirection.x = -forward->x;
		mDashDirection.y = 0.0f;
		mDashDirection.z = -forward->z;

		float square = (mDashDirection.x * mDashDirection.x) + (mDashDirection.z * mDashDirection.z);

		// Normalize defensively to ensure the vector snaps back to a perfect length of 1.0
		if (square > 0.0001f) {
			float oneOverSquareRoot = Engine::MathHelper::FastInverseSqrt(square);
			mDashDirection.x = mDashDirection.x * oneOverSquareRoot;
			mDashDirection.y = 0.0f;
			mDashDirection.z = mDashDirection.z * oneOverSquareRoot;
		}
		else {
			// Fallback: If forward is somehow pure vertical (0, 1, 0), 
			// default to a safe world-space backwards direction
			mDashDirection = Engine::Vector3(0.0f, 0.0f, -1.0f);
		}

		return false;
	}
	else {
		// update the timer
		mDashAnimationTimer += deltaTime;

		// position = position + (direction * speed * time)
		center->x += mDashDirection.x * backwardsDashVelocity * deltaTime;
		center->z += mDashDirection.z * backwardsDashVelocity * deltaTime;

		if (mDashAnimationTimer >= animationDuration) {
			mIsPerformingBackwardsDash = false;
			mIsBackwardsDashAnimationComplete = true;
			mDashAnimationTimer = 0.f;
		}

		return true;
	}
}

bool PlayerAnimator::GetIsBackwardsDashAnimationComplete() const {
	return mIsBackwardsDashAnimationComplete;
}

bool PlayerAnimator::GetIsRotationComplete() const {
	return mIsRotationComplete;
}

void PlayerAnimator::SetIsRotationComplete(bool value) {
	mIsRotationComplete = value;
}