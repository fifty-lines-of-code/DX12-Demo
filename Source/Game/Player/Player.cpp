#include "Player.h"

#include <cmath>
#include "../../Engine/Camera/Camera.h"
#include "../../Engine/Scene Manager/Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"

Player::Player() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity* entity) {
	if (entity == nullptr) { return; }

	mEntity = entity;

	UpdateEntityCenterAndRotationAndSetItToDirty();
}

void Player::Update(float deltaTime, const IInputSystem* const inputSystem, CameraForwardAndRightVectors forwardAndRightVectors) {
	// update the state of the action east (b or circle) button
	UpdateActionEastButtonState(
		deltaTime,
		inputSystem->GetButtonState(GameButton::ActionEast)
	);

	// move the player if controller demands it
	MoveAndRotatePlayer(
		deltaTime, 
		inputSystem->GetLeftStickX(),
		inputSystem->GetLeftStickY(),
		forwardAndRightVectors
	);
}

void Player::UpdateActionEastButtonState(float deltaTime, GameButtonState actionEastButtonState) {
	if (actionEastButtonState == GameButtonState::Unpressed) {
		mIsRunning = false;
		if (mTotalTimeBWasHeldDown > 0) {
			//we do nothing yet, but later perform backwards dash
			mPerformBackwardsDashIfNotRunning = true;
		}
		mTotalTimeBWasHeldDown = -1;
	}
	else {
		if (mTotalTimeBWasHeldDown == -1) {
			mTotalTimeBWasHeldDown = 0.f;
		}
		else {
			mTotalTimeBWasHeldDown += deltaTime;
			if (mTotalTimeBWasHeldDown > mTotalDurationPlayerHasToRunAfterPressingB) {
				mIsRunning = true;
			}
		}
	}
}

void Player::MoveAndRotatePlayer(float deltaTime, float leftStickX, float leftStickY, CameraForwardAndRightVectors forwardAndRightVectors) {

	DirectX::XMFLOAT3 movementForward = DirectX::XMFLOAT3();
	movementForward.x = leftStickY * forwardAndRightVectors.forward.x;
	movementForward.z = leftStickY * forwardAndRightVectors.forward.z;

	DirectX::XMFLOAT3 movementRight = DirectX::XMFLOAT3();
	movementRight.x = leftStickX * forwardAndRightVectors.right.x;
	movementRight.z = leftStickX * forwardAndRightVectors.right.z;

	DirectX::XMFLOAT3 movement;
	movement.x = movementForward.x + movementRight.x;
	movement.y = 0.f;
	movement.z = movementForward.z + movementRight.z;

	// normalize movement
	float movementLengthSquared = movement.x * movement.x + movement.z * movement.z;

	if (leftStickX != 0.0f || leftStickY != 0.0f) {

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

		float walkingSpeed = isWalkingSlow ? mPlayerSlowWalkingSpeed : mPlayerNormalWalkingSpeed;
		
		// if isRunning and NOT walking slow (so normal walking)
		// then run
		// otherwise walk the computed walking speed
		float speed = (mIsRunning && !isWalkingSlow) ? mPlayerRunningSpeed : walkingSpeed;
		float speedMultipliedByDelta = speed * deltaTime;

		// update center
		mCenter.x += movement.x * speedMultipliedByDelta;
		mCenter.y += movement.y * speedMultipliedByDelta;
		mCenter.z += movement.z * speedMultipliedByDelta;

		// update rotation
		RotatePlayer(deltaTime, movement);

		UpdateEntityCenterAndRotationAndSetItToDirty();
	}
	else {
		// only perform backwards dash if we are idle
		if (!mIsPerformingBackwardsDash) {
			if (mPerformBackwardsDashIfNotRunning) {
				mPerformBackwardsDashIfNotRunning = false;

				// normalize movement vector
				if (movementLengthSquared > 1.f) {
					// Use the Legendary Quake 3 Inverse Sq Root for the heck of it
					float oneOverMovementLengthSquared = MathHelper::FastInverseSqrt(movementLengthSquared);
					// only update x and z for now
					movement.x *= oneOverMovementLengthSquared;
					movement.z *= oneOverMovementLengthSquared;
				}
				mIsPerformingBackwardsDash = true;
				mDashStartPosition = mCenter;

				mDashTargetPosition.x = mCenter.x;
				mDashTargetPosition.y = .6f;
				// todo: for now we're dashing in the -z direction at all times
				// update to dash in the -FWD direction
				mDashTargetPosition.z = mCenter.z - mBackwardsDashDistance;

				mDashAnimationTimer = 0.f;
			}
		}
		else {
			mDashAnimationTimer += deltaTime;
			float t = std::fmin(mDashAnimationTimer/mBackwardsDashAnimationDuration, 1.f);
			mCenter.x = mDashStartPosition.x + (mDashTargetPosition.x - mDashStartPosition.x) * t;
			mCenter.z = mDashStartPosition.z + (mDashTargetPosition.z - mDashStartPosition.z) * t;

			if (mDashAnimationTimer >= mBackwardsDashAnimationDuration) {
				mIsPerformingBackwardsDash = false;
				mDashAnimationTimer = 0.f;
			}

			UpdateEntityCenterAndRotationAndSetItToDirty();
		}
	}
}

void Player::RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement) {
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
	mCurrentRotation += deltaRotation * mRotationSpeed * deltaTime;
}

DirectX::XMFLOAT4 Player::GetCenter() const {
	return DirectX::XMFLOAT4(mCenter.x, mCenter.y, mCenter.z, 1.f);
}

void Player::UpdateEntityCenterAndRotationAndSetItToDirty() {
	// update the center in entity
	mEntity->SetCenter(mCenter);

	// update rotation in entity
	mEntity->SetRotation(mCurrentRotation);

	// set isDirty to true
	mEntity->SetIsDirty(true);
}