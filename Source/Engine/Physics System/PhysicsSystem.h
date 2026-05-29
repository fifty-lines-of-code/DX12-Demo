#pragma once

#include "../Math/GeometryHelper.h"
#include <vector>

class Entity;

namespace Engine::EnginePhysics {

	class PhysicsSystem {
	public:
		PhysicsSystem() = default;
		~PhysicsSystem() = default;

		void ResolveEntityMovement(Entity& entity, const std::vector<const Entity*>& candidates);

	private:
		bool ResolveCollision(uint32_t entityID, const AABB& aabb, const std::vector<const Entity*>& candidates);
	};
}