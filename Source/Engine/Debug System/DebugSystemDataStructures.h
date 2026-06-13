#pragma once

#include "../Math/EngineMath.h"

namespace Engine::DebugSystem {

	struct DebugTextSlot {
		Vector4 Color;
		char Character = '\0';
	};

	struct DebugSystemPerCharacterData {
		Vector4 Color;
		Vector2 Position;
		uint32_t Ascii;
		uint32_t Padding0;

		// make sure to update DX12FrameResource and Debug Shader
		// when we update this
	};

	struct DebugSystemPerPassConstantBuffer {
		float WindowWidth;
		float WindowHeight;
		float QuadWidth = DebugLimits::QUAD_WIDTH;
		float QuadHeight = DebugLimits::QUAD_HEIGHT;
		float UV_CellWidth;
		float UV_CellHeight = 1.f;
		float Padding[2] = { 0.f, 0.f };

		// make sure to update DX12FrameResource and Debug Shader
		// when we update this
	};

	struct FontAtlasDesc {
		uint32_t TextureAtlasSlot;
		uint32_t TextureWidth;
		uint32_t TextureHeight;
		uint32_t CellWidth;
		uint32_t CellHeight;
		uint32_t GridColumns;
		uint32_t GridRows; 
	};
}