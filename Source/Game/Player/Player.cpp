#include "Player.h"

#include <cmath>
#include "../../Engine/Camera/Camera.h"
#include "../../Engine/Scene Manager/Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"

Player::Player() : mPlayerAnimator(mCurrentRotation) {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity* entity) {
	if (entity == nullptr) { return; }

	mEntity = entity;

	UpdateEntityCenterAndRotationAndSetItToDirty();
}

void Player::Update(float deltaTime, const IInputSystem* const inputSystem, CameraForwardAndRightVectors forwardAndRightVectors) {

	// update player's state
	mPlayerLogic.Update(deltaTime, inputSystem);

	// perform animation, if any
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();
	bool result = false;

	switch (mPlayerLogic.GetPlayerState()) {
	case PlayerState::Idle: 
	case PlayerState::PendingActionEast: return;
	case PlayerState::Walking:
	case PlayerState::Running:
	{
		DirectX::XMFLOAT3 movement = CalculateMovementVectorFrom(
			forwardAndRightVectors,
			leftStickX,
			leftStickY
		);
		float square = movement.x * movement.x + movement.y * movement.y;
		float speed = mPlayerLogic.GetWalkingRunningSpeed(square);
		result = mPlayerAnimator.MoveAndRotatePlayer(
			deltaTime,
			movement,
			&mCenter,
			speed,
			mPlayerLogic.mRotationSpeed
		);
		break;
	}
	case PlayerState::BackwardsDashing:
		result = mPlayerAnimator.PerformBackwardsDash(
			deltaTime, 
			&mCenter,
			mPlayerLogic.mBackwardsDashDistance,
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


DirectX::XMFLOAT3 Player::CalculateMovementVectorFrom(CameraForwardAndRightVectors forwardAndRightVectors, float leftStickX, float leftStickY) {
	// calculate movement vector
	DirectX::XMFLOAT3 movementForward = DirectX::XMFLOAT3();
	movementForward.x = leftStickY * forwardAndRightVectors.forward.x;
	movementForward.z = leftStickY * forwardAndRightVectors.forward.z;

	DirectX::XMFLOAT3 movementRight = DirectX::XMFLOAT3();
	movementRight.x = leftStickX * forwardAndRightVectors.right.x;
	movementRight.z = leftStickX * forwardAndRightVectors.right.z;

	DirectX::XMFLOAT3 movement;
	movement.x = movementForward.x + movementRight.x;
	// todo: for now y is 0.6 (const), but possible we may go up or down hill
	// so update when ready to handle shift in y
	movement.y = 0.6f;
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