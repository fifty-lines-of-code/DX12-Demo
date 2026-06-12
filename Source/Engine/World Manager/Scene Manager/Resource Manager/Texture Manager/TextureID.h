#pragma once

#include <cstdint>
#include "../../../../EngineConfig.h"

namespace Engine::EngineResources {

	enum class TextureID : uint32_t {
		WOOD_CRATE,
		COUNT,
		INVALID
	};
	static_assert((size_t)TextureID::COUNT < EngineConfig::EngineConfig::MAX_TEXTURES && "Number of Textures defined should be less than MAX_TEXTURES");
}