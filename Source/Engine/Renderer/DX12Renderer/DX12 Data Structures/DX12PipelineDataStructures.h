#pragma once

#include <array>
#include <cstdint>
#include "../DX12RendererConfig.h"
#include "../../IRenderer.h"
#include <vector>

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12OpaqueRenderItemPerSubMeshExecuteContext {
		uint8_t ID;
		uint32_t IndexCount;
		uint32_t StartIndexLocation;
		uint32_t BaseVertexLocation;
	};

	struct DX12OpaqueRenderItemExecuteContext {
		uint32_t ID;
		uint32_t MeshID;
		uint8_t SubMeshCount;
		const std::vector< DX12OpaqueRenderItemPerSubMeshExecuteContext> SubMeshExecuteContext;
	};

	struct DX12OpaquePipelinePassExecuteContext : public IPipelinePassExecuteContext {
		uint32_t NumberOfItems;
		uint8_t NumOfSubMeshesPerItem;
		uint32_t NumberOfMaterials;
		const DX12OpaqueRenderItemExecuteContext* RenderItems;
		RendererPipelinePass PipelinePass;
		RendererPipelinePass GetPipelinePassType() const override {
			return PipelinePass;
		}
	};
}