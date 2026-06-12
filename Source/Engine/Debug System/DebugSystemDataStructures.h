#pragma once

#include "../Math/EngineMath.h"
#include <string>

namespace Engine::DebugSystem {

	struct TextVertex {
		Vector2 Position;
		Vector2 TexCoord;
		Vector4 Color;
	};

	struct FontAtlasDesc {
		uint32_t TextureAtlasSlot;
		uint32_t TextureWidth;
		uint32_t TextureHeight;
		uint32_t CellWidth;
		uint32_t CellHeight;
		uint32_t GridColumns = 16;  // How many characters wide the font sheet image is
		uint32_t GridRows = 16;  // How many characters high the font sheet image is
	};

	struct DebugTextInstance {
		std::string Text;
		float X = 0.f;
		float Y = 0.f;
		Vector4 Color;
	};
}