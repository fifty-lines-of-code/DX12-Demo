#pragma once

#include "../../Physics System/CollisionResult.h"
#include "../Scene Manager/Entity/Entity.h"
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

	void SetEntity(Engine::EngineWorld::Entity& entity);
	void Update(
		float deltaTime,
		const IInputSystem* const inputSystem, 
		const Engine::BasisVectors& cameraBasisVectors
	);

	const Engine::Vector3& GetCenter() const;
	Engine::AABB CalculatePotentialFootprintAABB() const;
	void PostPhysicsUpdate(
		const Engine::EnginePhysics::CollisionResult&
	);

private:
	Engine::EngineWorld::Entity* mEntity = nullptr;
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