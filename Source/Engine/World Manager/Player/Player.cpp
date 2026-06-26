#include "Player.h"

#include <cmath>
#include "../Scene Manager/Entity/Entity.h"
#include "../../Physics System/PhysicsBody.h"

#pragma region Public

Player::Player() : mPlayerAnimator() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Engine::EngineWorld::Entity& entity) { mEntity = &entity; }

void Player::Update(
	float deltaTime, 
	const IInputSystem* const inputSystem, 
	const Engine::BasisVectors& cameraBasisVectors
) {
	// reset velocity intent
	Engine::EnginePhysics::PhysicsBody& physicsBody = mEntity->GetPhysicsBody();
	Engine::EngineWorld::EntityTransformData& transformData = mEntity->GetTransformData();

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
	case PlayerState::Walking:
	case PlayerState::Running:
	{
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
	{
		Engine::Vector3 forward = transformData.BasisVectors.Forward;
		forward.Normalize();

		physicsBody.VelocityIntent.x = -forward.x *  mPlayerLogic.mBackwardsDashVelocity * deltaTime;
		physicsBody.VelocityIntent.z = -forward.z * mPlayerLogic.mBackwardsDashVelocity * deltaTime;

		mPlayerAnimator.AnimateBackwardsDash(
			deltaTime,
			mPlayerLogic.mBackwardsDashAnimationDuration
		);

		if (mPlayerAnimator.GetIsBackwardsDashAnimationComplete()) {
			mPlayerLogic.SetPlayerState(PlayerState::Idle);
		}

		break;
	}

	default: break;
	}

	mEntity->SetIsDirty(true);
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

void Player::PostPhysicsUpdate(
	const Engine::EnginePhysics::CollisionResult& collisionResult
) {
	Engine::EngineWorld::EntityTransformData& transformData = mEntity->GetTransformData();

	transformData.Center = collisionResult.ProposedCenter;

	if (collisionResult.HasCollided() && 
		mPlayerLogic.GetPlayerState() == PlayerState::BackwardsDashing) {
		mPlayerLogic.SetPlayerState(PlayerState::Idle);
		mPlayerAnimator.ResetBackwardsDashAnimation();
	}

	mEntity->SetIsDirty(true);
}

#pragma endregion

#pragma region Private

void Player::UpdateBasisVectorsFromVisualRotation() {
	float currentRotation = mPlayerAnimator.GetRotation();

	Engine::EngineWorld::EntityTransformData& transformData = mEntity->GetTransformData();

	transformData.BasisVectors.Forward.x = std::sin(currentRotation);
	transformData.BasisVectors.Forward.y = 0.0f;
	transformData.BasisVectors.Forward.z = std::cos(currentRotation);

	transformData.BasisVectors.Right.x = std::cos(currentRotation);
	transformData.BasisVectors.Right.y = 0.0f;
	transformData.BasisVectors.Right.z = -std::sin(currentRotation);
}

void Player::CalculateMovementVector(
	float leftStickX, 
	float leftStickY, 
	const Engine::BasisVectors& cameraBasisVectors, 
	Engine::Vector3& movement
) {
	// 1. Isolate and flatten the camera's forward vector to the 2D ground plane
	Engine::Vector3 flatCamFwd = { cameraBasisVectors.Forward.x, 0.0f, cameraBasisVectors.Forward.z };
	flatCamFwd.Normalize();

	// 2. Isolate and flatten the camera's right vector to the 2D ground plane
	Engine::Vector3 flatCamRight = { cameraBasisVectors.Right.x, 0.0f, cameraBasisVectors.Right.z };
	flatCamRight.Normalize();

	// 3. Now safely blend horizontal basis vectors by the stick inputs
	movement.x = (leftStickY * flatCamFwd.x) + (leftStickX * flatCamRight.x);
	movement.y = 0.0f; // Stable flat ground line
	movement.z = (leftStickY * flatCamFwd.z) + (leftStickX * flatCamRight.z);
	movement.ClampMagnitude(1.f);
}

#pragma endregion