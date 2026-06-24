#pragma once

#include <array>
#include <cstdint>
#include "../DX12RendererConfig.h"
#include "../../IRenderer.h"
#include <vector>

namespace Engine::EngineRenderer::DX12Renderer {

	// Pipeline pass execute context
	struct DX12PipelinePassExecuteContext : 
		public IPipelinePassExecuteContext {
		uint32_t NumberOfItems;
		RendererPipelinePass PipelinePass;

		RendererPipelinePass GetPipelinePassType() const override {
			return PipelinePass;
		}
	};

	// opaque render item per sub mesh execute context
	struct DX12OpaqueRenderItemPerSubMeshExecuteContext {
		uint8_t ID;
		uint32_t IndexCount;
		uint32_t StartIndexLocation;
		uint32_t BaseVertexLocation;
	};

	// opaquep per render item execute context
	struct DX12OpaqueRenderItemExecuteContext {
		uint32_t ID;
		uint32_t MeshID;
		uint8_t SubMeshCount;
		std::vector<DX12OpaqueRenderItemPerSubMeshExecuteContext> SubMeshExecuteContext;
	};

	// opaque pass execute context
	struct DX12OpaquePipelinePassExecuteContext : 
		public DX12PipelinePassExecuteContext {
		uint8_t MaxNumSubMeshesPerItem;
		uint32_t NumberOfMaterials;
		const DX12OpaqueRenderItemExecuteContext* RenderItems;
	};
}