#pragma once

#include <cstdint>

namespace Engine::EngineConfig {

	class EngineConfig {
	public:
		static constexpr uint16_t MAX_ENTITIES = 3;
		static constexpr uint16_t MAX_TEXTURES = 128;
		static constexpr uint16_t MAX_MATERIALS = 256; 
		static constexpr uint8_t MAX_SUBMESHES_PER_MESH = 8;
		static constexpr uint8_t MAX_MIRROR_PLANES = 8;

		// epsilons
		static constexpr float PHYSICS_Y_EPSILON = 0.025f;
		static constexpr float MIRROR_PLANE_DISTANCE_EPSILON = 0.02f;
		static constexpr float MIRROR_PLANE_ANGLE_EPSILON = 0.1f;
	};
} 