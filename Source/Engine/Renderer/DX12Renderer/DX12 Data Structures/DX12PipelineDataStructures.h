#pragma once

#include <array>
#include <cstdint>

#include "../DX12RendererConfig.h"
#include "../../IRenderer.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12RenderItemExecuteContext {
		uint32_t ID;
		uint32_t IndexCount;
		uint32_t MeshID;
	};

	struct DX12PipelinePassExecuteContext : public IPipelinePassExecuteContext {
		uint32_t NumberOfItems;
		uint32_t NumberOfMaterials;
		const DX12RenderItemExecuteContext* RenderItems;
		RendererPipelinePass PipelinePass;
		RendererPipelinePass GetPipelinePassType() const override {
			return PipelinePass;
		}
	};
}