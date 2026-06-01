#pragma once

#include "CollisionResult.h"
#include "../Math/GeometryHelper.h"
#include <vector>

class Entity;

namespace Engine::EnginePhysics {

	class PhysicsSystem {
	public:
		PhysicsSystem() = default;
		~PhysicsSystem() = default;

		void ResolveEntityMovement(Entity& entity, const std::vector<const Entity*>& candidates, CollisionResult& collisionResult);

	private:
		bool ResolveCollision(uint32_t entityID, const AABB& aabb, const std::vector<const Entity*>& candidates);
	};
}