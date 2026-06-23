#pragma once

#include "CollisionResult.h"
#include "../World Manager/Scene Manager/Entity/Entity.h"
#include "../Math/GeometryHelper.h"
#include <vector>

class Entity;

namespace Engine::EnginePhysics {

	struct PhysicsEntity {
		AABB AABB;
		uint32_t ID = 0;
		float scaleY = 0.f;
		bool IsTerrain = false;
	};

	class PhysicsSystem {
	public:
		PhysicsSystem() = default;
		~PhysicsSystem() = default;

		void ResolveEntityMovement(
			EngineWorld::Entity& entity,
			const std::vector<PhysicsEntity>& candidates,
			float terrainGroundY,
			CollisionResult& collisionResult
		);

	private:
		bool ResolveCollision(
			uint32_t entityID,
			const AABB& aabb, 
			const std::vector<PhysicsEntity>& candidates
		);
	};
}