#include "PlayerAnimator.h"

#include "../../../Engine/Camera/Camera.h"
#include <cmath>

PlayerAnimator::PlayerAnimator(float currentRotation) :
	mCurrentRotation(currentRotation)
{}

PlayerAnimator::~PlayerAnimator() {}

bool PlayerAnimator::MoveAndRotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, DirectX::XMFLOAT3* center, float walkingRunningSpeed, float rotationSpeed) {
	// get the square so we can normalize movement if need be
	float movementLengthSquared = movement.x * movement.x + movement.z * movement.z;

	// check if walking slow or fast
	bool isWalkingSlow = true;
	if (movementLengthSquared >= 0.25f) {
		isWalkingSlow = false;
	}

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
	// todo: hardcoded for now. will need to take y into consideration when ready
	center->y = 0.6f;
	center->z += movement.z * speedMultipliedByDelta;

	// update rotation
	RotatePlayer(deltaTime, movement, rotationSpeed);

	return true;
}

bool PlayerAnimator::PerformBackwardsDash(float deltaTime, DirectX::XMFLOAT3* center, float backwardsDashDistance, float animationDuration) {
	if (!mIsPerformingBackwardsDash) {

		mIsPerformingBackwardsDash = true;
		mBackwardsDashAnimationHasCompleted = false;
		mDashStartPosition.x = center->x;
		mDashStartPosition.y = center->y;
		mDashStartPosition.z = center->z;

		mDashTargetPosition.x = center->x;
		mDashTargetPosition.y = .6f;
		// todo: for now we're dashing in the -z direction at all times
		// update to dash in the -FWD direction
		mDashTargetPosition.z = center->z - backwardsDashDistance;

		mDashAnimationTimer = 0.f;

		return false;
	}
	else {
		mDashAnimationTimer += deltaTime;
		float t = std::fmin(mDashAnimationTimer / animationDuration, 1.f);

		center->x = mDashStartPosition.x + (mDashTargetPosition.x - mDashStartPosition.x) * t;
		center->z = mDashStartPosition.z + (mDashTargetPosition.z - mDashStartPosition.z) * t;

		if (mDashAnimationTimer >= animationDuration) {
			mIsPerformingBackwardsDash = false;
			mBackwardsDashAnimationHasCompleted = true;
			mDashAnimationTimer = 0.f;
		}

		return true;
	}
}

bool PlayerAnimator::IsBackwardsDashAnimationComplete() const {
	return mBackwardsDashAnimationHasCompleted;
}

void PlayerAnimator::RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, float rotationSpeed) {
	// We have to send in x first and then z to conert again from 
	// Math's RH rule to DX12's LH coordinate rule
	// same with negating the result.
	// Rotation in Math is CCW and DX12 is CW
	float targetRotation = -std::atan2(movement.x, movement.z);
	float deltaRotation = targetRotation - mCurrentRotation;

	// we have to make sure we take the shortest rotation 
	// so rotate -90 instead of 270
	// to do that we subtract 2pi if delta is > pi
	// and add 2pi if delta is < -pi
	// since rotation values will accumulate, we do this over a loop

	while (deltaRotation > MathHelper::Pi) { deltaRotation -= MathHelper::Two_Pi; }

	while (deltaRotation < -MathHelper::Pi) { deltaRotation += MathHelper::Two_Pi; }

	// now we smoothly interpolate to the targetRotation
	mCurrentRotation += deltaRotation * rotationSpeed * deltaTime;
}
