#pragma once

#include "../Math/EngineMath.h"

namespace Engine::EnginePhysics {

	struct CollisionResult {
		float ProposedCenterX;
		float ProposedCenterY;
		float ProposedCenterZ;
		bool CollidedX = false;
		bool CollidedY = false;
		bool CollidedZ = false;

		// todo: handle Y
		// ignoring Y for now

		inline bool HasCollided() const noexcept {
			return CollidedX || CollidedZ;
		}
	};
}