#pragma once

#include "../../Physics System/CollisionResult.h"
#include "../../Math/Geometry.h"
#include "../.././../Engine/Input System/IInputSystem.h"
#include "../../../Engine/Math/MathHelper.h"
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

	const Engine::Vector3& GetCenter() const;
	Engine::AABB CalculatePotentialFootprintAABB() const;
	void PostPhysicsUpdate(const Engine::EnginePhysics::CollisionResult& collisioNResult);

private:
	Entity* mEntity = nullptr;
	PlayerLogic mPlayerLogic;
	PlayerAnimator mPlayerAnimator;

private:
	void UpdateBasisVectorsFromVisualRotation();
	void CalculateMovementVector(
		float leftStickX,
		float leftStickY,
		const Engine::BasisVectors& cameraBasisVectors,
		Engine::Vector3& movement
	);
};