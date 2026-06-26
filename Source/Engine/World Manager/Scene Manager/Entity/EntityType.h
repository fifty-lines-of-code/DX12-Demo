#pragma once

#include <cstdint>

namespace Engine {

	enum class EntityType : uint32_t {
		PLAYER,
		TERRAIN,
		FLOOR,
		WALL,
		MIRROR,
		COUNT,
		INVALID
	};
}