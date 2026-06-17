#pragma once

#include <cstdint>

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12RendererConfig {
		static constexpr uint32_t NUMBER_OF_FRAME_RESOURCES = 3;
		static constexpr uint32_t NUMBER_OF_SWAPCHAIN_BUFFERS = 2;
	};
}