#include "PlayerAnimator.h"

#include "../../../Engine/Camera/Camera.h"
#include <cmath>

PlayerAnimator::PlayerAnimator() {}

PlayerAnimator::~PlayerAnimator() {}

bool PlayerAnimator::MoveAndRotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, DirectX::XMFLOAT3* center, float* currentRotation, float walkingRunningSpeed, float rotationSpeed, float movementLengthSquared) {

	if (movementLengthSquared > 1.f) {
		// Use the Legendary Quake 3 Inverse Sq Root for the heck of it
		float oneOverMovementLengthSquared = MathHelper::FastInverseSqrt(movementLengthSquared);
		// only update x and z for now
		movement.x *= oneOverMovementLengthSquared;
		movement.z *= oneOverMovementLengthSquared;
	}

	float speedMultipliedByDelta = walkingRunningSpeed * deltaTime;

	// update center
	center->x += movement.x * speedMultipliedByDelta;
	// todo: hardcoded for now. will take y into consideration when ready
	center->y = 0.6f;
	center->z += movement.z * speedMultipliedByDelta;

	// update rotation
	RotatePlayer(deltaTime, movement, rotationSpeed, currentRotation);

	return true;
}

bool PlayerAnimator::PerformBackwardsDash(
	float deltaTime, 
	DirectX::XMFLOAT3* center, 
	DirectX::XMFLOAT3* const forward,
	float backwardsDashVelocity,
	float animationDuration
) {
	if (!mIsPerformingBackwardsDash) {
		// Frame 1: Setup the trajectory
		mIsPerformingBackwardsDash = true;
		mIsBackwardsDashAnimationComplete = false;
		mDashAnimationTimer = 0.f;

		// Calculate and store the direction once (flipped forward vector)
		// We flatten the Y axis to keep the dash strictly horizontal
		mDashDirection.x = -forward->x;
		mDashDirection.y = 0.0f;
		mDashDirection.z = -forward->z;

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

bool PlayerAnimator::IsBackwardsDashAnimationComplete() const {
	return mIsBackwardsDashAnimationComplete;
}

void PlayerAnimator::RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, float rotationSpeed, float* currentRotation) {
	// We have to send in x first and then z to conert again from 
	// Math's RH rule to DX12's LH coordinate rule
	// same with negating the result.
	// Rotation in Math is CCW and DX12 is CW
	float targetRotation = -std::atan2(movement.x, movement.z);
	float deltaRotation = targetRotation - *currentRotation;

	// we have to make sure we take the shortest rotation 
	// so rotate -90 instead of 270
	// to do that we subtract 2pi if delta is > pi
	// and add 2pi if delta is < -pi
	// since rotation values will accumulate, we do this over a loop

	while (deltaRotation > MathHelper::Pi) { deltaRotation -= MathHelper::Two_Pi; }

	while (deltaRotation < -MathHelper::Pi) { deltaRotation += MathHelper::Two_Pi; }

	// now we smoothly interpolate to the targetRotation
	*currentRotation += deltaRotation * rotationSpeed * deltaTime;
}