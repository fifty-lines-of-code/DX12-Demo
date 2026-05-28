#pragma once

#include <cstdint>

struct WorldDimensions {
	// spans -1024 to 1024 in each direction
	static constexpr float World_Size = 2048;
	static constexpr float World_Half_Size = World_Size * 0.5;
};