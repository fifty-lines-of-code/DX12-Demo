#include "PhysicsSystem.h"

#include "../World Manager/Scene Manager/Entity/Entity.h"

namespace Engine::EnginePhysics {
	void PhysicsSystem::ResolveEntityMovement(
		Entity& entity, 
		const std::vector<const Entity*>& candidates,
		CollisionResult& collisionResult
	) {
		PhysicsBody& physicsBody = entity.GetPhysicsBody();

		// Set the proposed center to current center
		collisionResult.ProposedCenterX = physicsBody.Center.x;
		collisionResult.ProposedCenterY = physicsBody.Center.y;
		collisionResult.ProposedCenterZ = physicsBody.Center.z;

		if (physicsBody.VelocityIntent.x == 0.f &&
			physicsBody.VelocityIntent.z == 0.f) {
			return;
		}

		uint32_t entityID = entity.GetID();
		// store the safe Center
		float safeCenterX = physicsBody.Center.x;
		float safeCenterZ = physicsBody.Center.z;

		// resolve movement in the x direction
		if (physicsBody.VelocityIntent.x != 0.f) {
			AABB potentialAABB;
			Vector3 testCenter = Vector3(safeCenterX, physicsBody.Center.y, safeCenterZ);
			testCenter.x += physicsBody.VelocityIntent.x;

			potentialAABB.Min = {
				testCenter.x + physicsBody.LocalAABB.Min.x,
				testCenter.y + physicsBody.LocalAABB.Min.y,
				testCenter.z + physicsBody.LocalAABB.Min.z
			};
			potentialAABB.Max = {
				testCenter.x + physicsBody.LocalAABB.Max.x,
				testCenter.y + physicsBody.LocalAABB.Max.y,
				testCenter.z + physicsBody.LocalAABB.Max.z
			};

			bool collidedX = ResolveCollision(
				entityID, 
				potentialAABB, 
				candidates
			);

			if (collidedX) {
				physicsBody.VelocityIntent.x = 0.f;
				collisionResult.CollidedX = true;
			}
			else {
				safeCenterX = testCenter.x;
			}
		}

		// resolve movement in the z direction
		if (physicsBody.VelocityIntent.z != 0.f) {
			AABB potentialAABB;
			Vector3 testCenter = Vector3(safeCenterX, physicsBody.Center.y, safeCenterZ);
			testCenter.z += physicsBody.VelocityIntent.z;

			potentialAABB.Min = {
				testCenter.x + physicsBody.LocalAABB.Min.x,
				testCenter.y + physicsBody.LocalAABB.Min.y,
				testCenter.z + physicsBody.LocalAABB.Min.z
			};
			potentialAABB.Max = {
				testCenter.x + physicsBody.LocalAABB.Max.x,
				testCenter.y + physicsBody.LocalAABB.Max.y,
				testCenter.z + physicsBody.LocalAABB.Max.z
			};

			bool collidedZ = ResolveCollision(
				entityID,
				potentialAABB,
				candidates
			);

			if (collidedZ) {
				physicsBody.VelocityIntent.z = 0;
				collisionResult.CollidedZ = true;
			}
			else {
				safeCenterZ = testCenter.z;
			}
		}

		// commit the movement
		collisionResult.ProposedCenterX = safeCenterX;
		collisionResult.ProposedCenterZ = safeCenterZ;
	}

	bool PhysicsSystem::ResolveCollision(
		uint32_t entityID,
		const AABB& aabb,
		const std::vector<const Entity*>& candidates
	) {

		for (const Entity* candidate : candidates) {
			if (entityID == candidate->GetID()) { continue; }

			if (GeometryHelper::AABBIntersect(aabb, candidate->GetAABB())) {
				return true;
			}
		}

		return false;
	}
}