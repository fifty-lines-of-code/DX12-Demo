#include "Player.h"

#include "../../Engine/Scene Manager/Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"

Player::Player() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity* entity) {
	if (entity == nullptr) { return; }

	mEntity = entity;

	UpdateEntityCenterAndSetItToDirty();
}

void Player::Update(float deltaTime, const IInputSystem* const inputSystem) {
	// move the player if controller demands it
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();
	MovePlayer(deltaTime, leftStickX, leftStickY);
}

void Player::MovePlayer(float deltaTime, float leftStickX, float leftStickY) {
	if (leftStickX != 0.0f || leftStickY != 0.0f) {
		// 1. Calculate the length of the input vector to check for diagonals
		float lengthSquared = (leftStickX * leftStickX) + (leftStickY * leftStickY);

		float dirX = leftStickX;
		// Mapping stick Y input over to our 3D world Z axis for now until
		// we handle rotation
		// todo
		float dirZ = leftStickY;

		// 2. Normalize direction
		if (lengthSquared > 1.0f) {
			// using the Quake3 copy-paste for the heck of it
			float oneOverLengthSquared = MathHelper::FastInverseSqrt(lengthSquared);
			// todo: switch back to oneOverLengthSquared = 1/lengthSquared;
			dirX *= oneOverLengthSquared;
			dirZ *= oneOverLengthSquared;
		}

		mCenter.x += dirX * mPlayerMovementSpeed * deltaTime;
		mCenter.z += dirZ * mPlayerMovementSpeed * deltaTime;

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