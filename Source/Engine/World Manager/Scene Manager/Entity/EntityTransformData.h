#pragma once
#include "../../../Math/Vector.h"

namespace Engine::EngineWorld {

	struct EntityTransformData {
		Vector3 Center;
		Vector3 Scale;
		
		// updraged to euler angles or quaternion later
		// current validated spatial state
		BasisVectors BasisVectors;
	};
}