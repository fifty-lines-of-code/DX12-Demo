#include "Player.h"

#include "../../Engine/Scene Manager/Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"

Player::Player(Entity* entity) : mEntity(entity) {}

Player::~Player() { mEntity = nullptr; }

void Player::Update(float deltaTime, const IInputSystem* const inputSystem) {
	// move the player if controller demands it
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();
	MovePlayer(deltaTime, leftStickX, leftStickY);
}

void Player::MovePlayer(float deltaTime, float leftStickX, float leftStickY) {

	if (leftStickX != 0.0f || leftStickY != 0.0f) {

		// 2. Calculate the length of the input vector to check for diagonals
		float lengthSquared = (leftStickX * leftStickX) + (leftStickY * leftStickY);

		float dirX = leftStickX;
		// Mapping stick Y input over to our 3D world Z axis for now until
		// we handle rotation
		// todo
		float dirZ = leftStickY;

		// 3. Normalize direction
		if (lengthSquared > 1.0f) {
			// using the Quake3 copy-paste for the heck of it
			float oneOverLengthSquared = MathHelper::FastInverseSqrt(lengthSquared);
			// todo: switch back to oneOverLengthSquared = 1/lengthSquared;
			dirX *= oneOverLengthSquared;
			dirZ *= oneOverLengthSquared;
		}

		DirectX::XMFLOAT3 center = mEntity->GetCenter();

		center.x += dirX * mPlayerMovementSpeed * deltaTime;
		center.z += dirZ * mPlayerMovementSpeed * deltaTime;

		// update the center in entity
		mEntity->SetCenter(center);

		// set isDirty to true
		mEntity->SetIsDirty(true);
	}
}