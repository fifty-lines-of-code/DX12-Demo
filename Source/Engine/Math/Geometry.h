#pragma once

#include "EngineMath.h"

namespace Engine {

	struct Vertex {
		Vector3 Position;
		Vector3 Normal;
	};

	struct AABB {
		Vector3 Min;
		Vector3 Max;

		AABB() : AABB(Vector3(), Vector3()) {}
		AABB(Vector3 min, Vector3 max) : Min(min), Max(max) {}
	};
}