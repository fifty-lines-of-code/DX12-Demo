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
	};
} 