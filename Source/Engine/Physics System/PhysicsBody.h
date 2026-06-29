#pragma once

#include "../Math/EngineMath.h"
#include "../Math/GeometryHelper.h"

namespace Engine::EnginePhysics {

	struct PhysicsBody {
		// transient simulation state
		Vector3 VelocityIntent;

		// bounds
		AABB LocalAABB;
		AABB WorldAABB;

		void UpdateWorldAABB(
			const Matrix4x4& worldMatrix
		) {
			GeometryHelper::CalculateAABB(
				LocalAABB,
				worldMatrix, 
				WorldAABB
			);
		}
	};
}