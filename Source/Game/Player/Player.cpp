#include "Player.h"

#include "../../Engine/Camera/Camera.h"
#include "../../Engine/Scene Manager/Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"

Player::Player() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity* entity) {
	if (entity == nullptr) { return; }

	mEntity = entity;

	UpdateEntityCenterAndSetItToDirty();
}

void Player::Update(float deltaTime, const IInputSystem* const inputSystem, CameraForwardAndRightVectors forwardAndRightVectors) {
	// move the player if controller demands it
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();
	MovePlayer(deltaTime, leftStickX, leftStickY, forwardAndRightVectors);
}

void Player::MovePlayer(float deltaTime, float leftStickX, float leftStickY, CameraForwardAndRightVectors forwardAndRightVectors) {
	if (leftStickX != 0.0f || leftStickY != 0.0f) {

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
		if (movementLengthSquared > 1.f) {
			// Use the Legendary Quake 3 Inverse Sq Root for the heck of it
			float oneOverMovementLengthSquared = MathHelper::FastInverseSqrt(movementLengthSquared);
			// only update x and z for now
			movement.x *= oneOverMovementLengthSquared;
			movement.z *= oneOverMovementLengthSquared;
		}
		
		float speedMultipliedByDelta = mPlayerMovementSpeed * deltaTime;

		mCenter.x += movement.x * speedMultipliedByDelta;
		mCenter.y += movement.y * speedMultipliedByDelta;
		mCenter.z += movement.z * speedMultipliedByDelta;

		UpdateEntityCenterAndSetItToDirty();
	}
}

DirectX::XMFLOAT4 Player::GetCenter() const {
	return DirectX::XMFLOAT4(mCenter.x, mCenter.y, mCenter.z, 1.f);
}

void Player::UpdateEntityCenterAndSetItToDirty() {
	// update the center in entity
	mEntity->SetCenter(mCenter);

	// set isDirty to true
	mEntity->SetIsDirty(true);

}