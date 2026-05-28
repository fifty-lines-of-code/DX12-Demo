#pragma once

#include "../../../Engine/Math/MathHelper.h"
#include "../.././../Engine/Input System/IInputSystem.h"
#include "Player Animator/PlayerAnimator.h"
#include "Player Logic/PlayerLogic.h"

struct BasisVectors;
class Entity;

class Player {
public:
	Player();
	~Player();

	void SetEntity(Entity& entity);
	void Update(
		float deltaTime,
		const IInputSystem* const inputSystem, 
		const Engine::BasisVectors& cameraBasisVectors
	);

	void CalculateNewPotentialCenter(
		float deltaTime, 
		const IInputSystem* const inputSystem, 
		const Engine::BasisVectors& cameraBasisVectors
	);

	const Engine::Vector3& GetCenter() const;

private:
	Entity* mEntity = nullptr;
	PlayerLogic mPlayerLogic;
	PlayerAnimator mPlayerAnimator;

	Engine::Vector3 mCenter = Engine::Vector3(0.f, 0.6f, .5f);
	float mCurrentRotation = Engine::MathHelper::ConvertToRadians(0);
	Engine::Vector3 mNewPotentialCenter = mCenter;

	// basis vectors
	Engine::BasisVectors mBasisVectors;

private:
	void UpdateBasisVectorsFromCurrentRotation();
	void CalculateMovementVector(
		float leftStickX,
		float leftStickY,
		const Engine::BasisVectors& cameraBasisVectors,
		Engine::Vector3& movement
	);
	void UpdateEntityCenterAndBasisVectorsAndSetItToDirty();
};