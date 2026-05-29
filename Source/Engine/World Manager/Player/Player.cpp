#include "Player.h"

#include <cmath>
#include "../Scene Manager/Entity/Entity.h"
#include "../../Physics System/PhysicsBody.h"

#pragma region Public

Player::Player() : mPlayerAnimator() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity& entity) { mEntity = &entity; }

void Player::Update(
	float deltaTime, 
	const IInputSystem* const inputSystem, 
	const Engine::BasisVectors& cameraBasisVectors
) {
	// reset velocity intent
	Engine::EnginePhysics::PhysicsBody& physicsBody = mEntity->GetPhysicsBody();
	physicsBody.VelocityIntent.Reset();

	// update player's state
	mPlayerLogic.Update(deltaTime, inputSystem);

	// calculate movement intent
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();

	Engine::Vector3 movement;
	CalculateMovementVector(
		leftStickX,
		leftStickY,
		cameraBasisVectors,
		movement
	);

	switch (mPlayerLogic.GetPlayerState()) {
	case PlayerState::Idle: 
	case PlayerState::PendingActionEast: break;
	case PlayerState::BeginRotating:
	{
		mPlayerAnimator.SetIsRotationComplete(false);
		mPlayerLogic.SetPlayerState(PlayerState::Rotating);
		// intentional fallthrough to start rotating this frame
	}
	case PlayerState::Rotating:
	{
		mPlayerAnimator.UpdateVisualRotation(
			deltaTime,
			movement,
			mPlayerLogic.mRotationSpeed
		);

		UpdateBasisVectorsFromVisualRotation();

		if (mPlayerAnimator.GetIsRotationComplete()) {
			mPlayerLogic.SetPlayerState(PlayerState::EndRotating);
		}
		break;
	}
	case PlayerState::Walking:
	case PlayerState::Running:
	{
		// update the visual rotation
		mPlayerAnimator.UpdateVisualRotation(
			deltaTime,
			movement,
			mPlayerLogic.mRotationSpeed
		);
		UpdateBasisVectorsFromVisualRotation();

		float speed = mPlayerLogic.GetWalkingRunningSpeed();
		physicsBody.VelocityIntent.x = movement.x * speed * deltaTime;
		physicsBody.VelocityIntent.z = movement.z * speed * deltaTime;

		break;
	}
	case PlayerState::BackwardsDashing:
		// todo
		break;
	}

	mEntity->SetIsDirty(true);
}

const Engine::Vector3& Player::GetCenter() const {
	return mEntity->GetPhysicsBody().Center;
}

Engine::AABB Player::CalculatePotentialFootprintAABB() const {
	const Engine::EnginePhysics::PhysicsBody& physicsBody = mEntity->GetPhysicsBody();
	Engine::AABB broadphase = physicsBody.WorldAABB;

	// x
	if (physicsBody.VelocityIntent.x > 0) {
		broadphase.Max.x += physicsBody.VelocityIntent.x;
	}
	else {
		broadphase.Min.x += physicsBody.VelocityIntent.x;
	}

	// z
	if (physicsBody.VelocityIntent.z > 0) {
		broadphase.Max.z += physicsBody.VelocityIntent.z;
	}
	else {
		broadphase.Min.z += physicsBody.VelocityIntent.z;
	}

	return broadphase;
}

#pragma endregion

#pragma region Private

void Player::UpdateBasisVectorsFromVisualRotation() {
	float currentRotation = mPlayerAnimator.GetRotation();

	Engine::EnginePhysics::PhysicsBody& physicsBpdy = mEntity->GetPhysicsBody();

	physicsBpdy.BasisVectors.forward.x = std::sin(currentRotation);
	physicsBpdy.BasisVectors.forward.y = 0.0f;
	physicsBpdy.BasisVectors.forward.z = std::cos(currentRotation);

	physicsBpdy.BasisVectors.right.x = std::cos(currentRotation);
	physicsBpdy.BasisVectors.right.y = 0.0f;
	physicsBpdy.BasisVectors.right.z = -std::sin(currentRotation);
}

void Player::CalculateMovementVector(
	float leftStickX, 
	float leftStickY, 
	const Engine::BasisVectors& cameraBasisVectors, 
	Engine::Vector3& movement
) {
	// 1. Isolate and flatten the camera's forward vector to the 2D ground plane
	Engine::Vector3 flatCamFwd = { cameraBasisVectors.forward.x, 0.0f, cameraBasisVectors.forward.z };
	flatCamFwd.Normalize();

	// 2. Isolate and flatten the camera's right vector to the 2D ground plane
	Engine::Vector3 flatCamRight = { cameraBasisVectors.right.x, 0.0f, cameraBasisVectors.right.z };
	flatCamRight.Normalize();

	// 3. Now safely blend your pristine, full-strength horizontal basis vectors by the stick inputs
	movement.x = (leftStickY * flatCamFwd.x) + (leftStickX * flatCamRight.x);
	movement.y = 0.0f; // Stable flat ground line
	movement.z = (leftStickY * flatCamFwd.z) + (leftStickX * flatCamRight.z);
	movement.ClampMagnitude(1.f);
}

#pragma endregion