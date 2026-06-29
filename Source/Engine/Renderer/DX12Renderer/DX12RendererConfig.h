#pragma once

#include <cstdint>
#include "../IRenderer.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12RendererConfig {
		static constexpr uint32_t NUMBER_OF_SWAPCHAIN_BUFFERS = 2;
		static constexpr uint32_t NUMBER_OF_FRAME_RESOURCES = 3;
		static constexpr uint32_t MAX_TEXTURES_IN_SHADERS = 256;
		static constexpr uint8_t MAX_NUMBER_OF_PASSES = (uint8_t)RendererPipelinePass::COUNT;
		static constexpr uint32_t MAX_ITEMS_PER_PASS = 128;
		static constexpr uint16_t CONSTANT_BUFFER_SHOULD_BE_MULTIPLES_OF = 256;
	};
}