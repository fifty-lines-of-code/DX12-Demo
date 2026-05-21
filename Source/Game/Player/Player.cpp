#include "Player.h"

#include <cmath>
#include "../../Engine/Camera/Camera.h"
#include "../../Engine/Scene Manager/Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"

Player::Player() : mPlayerAnimator() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity* entity) {
	if (entity == nullptr) { return; }

	mEntity = entity;

	UpdateEntityCenterAndRotationAndSetItToDirty();
}

void Player::Update(float deltaTime, const IInputSystem* const inputSystem, CameraForwardAndRightVectors forwardAndRightVectors) {

	// update player's state
	mPlayerLogic.Update(deltaTime, inputSystem);

	// now handle the new state
	bool result = false;

	switch (mPlayerLogic.GetPlayerState()) {
	case PlayerState::Idle: 
	case PlayerState::PendingActionEast: return;
	case PlayerState::Walking:
	case PlayerState::Running:
	{
		float leftStickX = inputSystem->GetLeftStickX();
		float leftStickY = inputSystem->GetLeftStickY();
		DirectX::XMFLOAT3 movement;

		CalculateMovementVectorFrom(
			forwardAndRightVectors,
			leftStickX,
			leftStickY,
			movement
		);

		float square = movement.x * movement.x + movement.y * movement.y;
		float speed = mPlayerLogic.GetWalkingRunningSpeed(square);

		result = mPlayerAnimator.MoveAndRotatePlayer(
			deltaTime,
			movement,
			&mCenter,
			&mCurrentRotation,
			speed,
			mPlayerLogic.mRotationSpeed,
			square
		);
		break;
	}
	case PlayerState::BackwardsDashing:
		result = mPlayerAnimator.PerformBackwardsDash(
			deltaTime, 
			&mCenter,
			&mForward,
			mPlayerLogic.mBackwardsDashVelocity,
			mPlayerLogic.mBackwardsDashAnimationDuration
		);
		// after animation finishes, set player state to idle
		if (mPlayerAnimator.IsBackwardsDashAnimationComplete()) {
			mPlayerLogic.SetPlayerState(PlayerState::Idle);
		}
		break;
	}

	if (result) {
		UpdateEntityCenterAndRotationAndSetItToDirty();
	}
}

DirectX::XMFLOAT4 Player::GetCenter() const {
	return DirectX::XMFLOAT4(mCenter.x, mCenter.y, mCenter.z, 1.f);
}


DirectX::XMFLOAT3 Player::CalculateMovementVectorFrom(CameraForwardAndRightVectors forwardAndRightVectors, float leftStickX, float leftStickY, DirectX::XMFLOAT3& movement) {
	// calculate movement vector
	DirectX::XMFLOAT3 movementForward = DirectX::XMFLOAT3();
	movementForward.x = leftStickY * forwardAndRightVectors.forward.x;
	movementForward.z = leftStickY * forwardAndRightVectors.forward.z;

	DirectX::XMFLOAT3 movementRight = DirectX::XMFLOAT3();
	movementRight.x = leftStickX * forwardAndRightVectors.right.x;
	movementRight.z = leftStickX * forwardAndRightVectors.right.z;

	movement.x = movementForward.x + movementRight.x;
	// todo: for now y is 0.0 (const), but possible we may go up or down hill
	// so update when ready to handle shift in y
	movement.y = 0.0f;
	movement.z = movementForward.z + movementRight.z;

	return movement;
}

void Player::UpdateEntityCenterAndRotationAndSetItToDirty() {
	// update the center in entity
	mEntity->SetCenter(mCenter);

	// update rotation in entity
	mEntity->SetRotation(mCurrentRotation);

	// set isDirty to true
	mEntity->SetIsDirty(true);
}