#pragma once

#include <cstdint>

namespace Engine {
	struct WorldDimensions {
		// spans -1024 to 1024 in each direction
		// assumed unit is "meter"
		// so world size is roughly a cube with 2km for l, w, d

		static constexpr uint16_t World_Size = 2048;
		static constexpr uint16_t World_Half_Size = World_Size / 2;
	};
}