#pragma once

#include "EngineMath.h"

namespace Engine {

	struct Vertex {
		Vector3 Position;
		Vector4 Color;
	};

	struct AABB {
		Vector3 Min;
		Vector3 Max;

		AABB() : AABB(Vector3(), Vector3()) {}
		AABB(Vector3 min, Vector3 max) : Min(min), Max(max) {}
	};
}