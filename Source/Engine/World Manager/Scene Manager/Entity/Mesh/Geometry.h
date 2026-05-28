#pragma once

#include "../../../../Math/EngineMath.h"

namespace Engine {

	struct Vertex {
		Engine::Vector3 Position = Engine::Vector3::Zero();
		Engine::Vector4 Color = Engine::Vector4::Zero();
	};
}