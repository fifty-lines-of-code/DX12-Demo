#include "Player.h"

#include <cmath>
#include "../Scene Manager/Entity/Entity.h"

Player::Player() : mPlayerAnimator() {}

Player::~Player() { mEntity = nullptr; }

void Player::SetEntity(Entity& entity) {
	mEntity = &entity;

	UpdateEntityCenterAndBasisVectorsAndSetItToDirty();
}

void Player::Update(
	float deltaTime, 
	const IInputSystem* const inputSystem, 
	const Engine::BasisVectors& cameraBasisVectors
) {

	// update player's state
	mPlayerLogic.Update(deltaTime, inputSystem);

	// now handle the new state
	bool shouldUpdateEntity = false;

	switch (mPlayerLogic.GetPlayerState()) {
	case PlayerState::Idle: 
	case PlayerState::PendingActionEast: return;
	case PlayerState::BeginRotating:
	{
		mPlayerAnimator.SetIsRotationComplete(false);
		mPlayerLogic.SetPlayerState(PlayerState::Rotating);
		break;
	}
	case PlayerState::Rotating:
	{
		shouldUpdateEntity = true;

		float leftStickX = inputSystem->GetLeftStickX();
		float leftStickY = inputSystem->GetLeftStickY();
		
		float movementSqLength = (leftStickX * leftStickX) + (leftStickY * leftStickY);
		if (movementSqLength <= 0.01f) {
			mPlayerLogic.SetPlayerState(PlayerState::Idle);
			break;
		}

		Engine::Vector3 movement;
		CalculateMovementVector(
			leftStickX,
			leftStickY,
			cameraBasisVectors,
			movement
		);

		mPlayerAnimator.RotatePlayer(
			deltaTime,
			movement,
			mPlayerLogic.mRotationSpeed,
			&mCurrentRotation
		);

		UpdateBasisVectorsFromCurrentRotation();

		if (mPlayerAnimator.GetIsRotationComplete()) {
			mPlayerLogic.SetPlayerState(PlayerState::EndRotating);
		}
		break;
	}
	case PlayerState::Walking:
	case PlayerState::Running:
	{
		float leftStickX = inputSystem->GetLeftStickX();
		float leftStickY = inputSystem->GetLeftStickY();

		Engine::Vector3 movement;
		CalculateMovementVector(
			leftStickX,
			leftStickY,
			cameraBasisVectors,
			movement
		);

		float speed = mPlayerLogic.GetWalkingRunningSpeed();

		shouldUpdateEntity = mPlayerAnimator.MoveAndRotatePlayer(
			deltaTime,
			movement,
			mCenter,
			&mCurrentRotation,
			speed,
			mPlayerLogic.mRotationSpeed
		);

		UpdateBasisVectorsFromCurrentRotation();

		break;
	}
	case PlayerState::BackwardsDashing:
		shouldUpdateEntity = mPlayerAnimator.PerformBackwardsDash(
			deltaTime,
			mCenter,
			mBasisVectors.forward,
			mPlayerLogic.mBackwardsDashVelocity,
			mPlayerLogic.mBackwardsDashAnimationDuration
		);

		// after animation finishes, set player state to idle
		if (mPlayerAnimator.GetIsBackwardsDashAnimationComplete()) {
			mPlayerLogic.SetPlayerState(PlayerState::Idle);
		}
		break;
	}

	if (shouldUpdateEntity) {
		UpdateEntityCenterAndBasisVectorsAndSetItToDirty();
	}
}

void Player::CalculateNewPotentialCenter(
	float deltaTime, 
	const IInputSystem* const inputSystem, 
	const Engine::BasisVectors& cameraBasisVectors
) {
	mNewPotentialCenter = mCenter;

	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();

	switch (mPlayerLogic.GetPlayerState()) {
	case PlayerState::Walking:
	case PlayerState::Running:
	{
		Engine::Vector3 movement;
		CalculateMovementVector(
			leftStickX,
			leftStickY,
			cameraBasisVectors,
			movement
		);

		float speed = mPlayerLogic.GetWalkingRunningSpeed();
		// update new potential center
		mNewPotentialCenter.x += movement.x * speed * deltaTime;
		mNewPotentialCenter.z += movement.z * speed * deltaTime;

		mEntity->SetPotentialCenter(mNewPotentialCenter);

		break;
	}
	case PlayerState::BackwardsDashing:
			// todo
		break;
	default: break;
	}
}

const Engine::Vector3& Player::GetCenter() const {
	return mCenter;
}

void Player::UpdateBasisVectorsFromCurrentRotation() {
	mBasisVectors.forward.x = std::sin(mCurrentRotation);
	mBasisVectors.forward.y = 0.0f;
	mBasisVectors.forward.z = std::cos(mCurrentRotation);

	mBasisVectors.right.x = std::cos(mCurrentRotation);
	mBasisVectors.right.y = 0.0f;
	mBasisVectors.right.z = -std::sin(mCurrentRotation);
}

void Player::CalculateMovementVector(
	float leftStickX, 
	float leftStickY, 
	const Engine::BasisVectors& cameraBasisVectors, 
	Engine::Vector3& movement
) {
	// 1. Isolate and flatten the camera's forward vector to the 2D ground plane
	Engine::Vector3 flatCamFwd = { cameraBasisVectors.forward.x, 0.0f, cameraBasisVectors.forward.z };
	float fwdSqLen = (flatCamFwd.x * flatCamFwd.x) + (flatCamFwd.z * flatCamFwd.z);

	// Re-normalize it so "Forward" is always a full 1.0 magnitude along the dirt
	if (fwdSqLen > 0.0001f) {
		float invLen = Engine::MathHelper::FastInverseSqrt(fwdSqLen);
		flatCamFwd.x *= invLen;
		flatCamFwd.z *= invLen;
	}

	// 2. Isolate and flatten the camera's right vector to the 2D ground plane
	DirectX::XMFLOAT3 flatCamRight = { cameraBasisVectors.right.x, 0.0f, cameraBasisVectors.right.z };
	float rgtSqLen = (flatCamRight.x * flatCamRight.x) + (flatCamRight.z * flatCamRight.z);

	// Re-normalize it right away
	if (rgtSqLen > 0.0001f) {
		float invLen = Engine::MathHelper::FastInverseSqrt(rgtSqLen);
		flatCamRight.x *= invLen;
		flatCamRight.z *= invLen;
	}

	// 3. Now safely blend your pristine, full-strength horizontal basis vectors by the stick inputs
	movement.x = (leftStickY * flatCamFwd.x) + (leftStickX * flatCamRight.x);
	movement.y = 0.0f; // Stable flat ground line
	movement.z = (leftStickY * flatCamFwd.z) + (leftStickX * flatCamRight.z);

	// 4. Smooth out the diagonal corner speed boost if stick is pushed into a corner (e.g., 1.414 length)
	float squared = (movement.x * movement.x) + (movement.z * movement.z);
	if (squared > 1.0f) {
		float oneOverSquareRoot = Engine::MathHelper::FastInverseSqrt(squared);
		movement.x *= oneOverSquareRoot;
		movement.z *= oneOverSquareRoot;
	}
}

void Player::UpdateEntityCenterAndBasisVectorsAndSetItToDirty() {
	// update the center in entity
	mEntity->SetCenter(mCenter);

	// update rotation in entity
	mEntity->SetBasisVectors(&mBasisVectors);

	// set isDirty to true
	mEntity->SetIsDirty(true);
}