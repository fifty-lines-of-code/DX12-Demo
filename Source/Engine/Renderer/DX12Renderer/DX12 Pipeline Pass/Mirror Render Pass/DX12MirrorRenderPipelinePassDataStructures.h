#pragma once

#include <array>
#include <cstdint>
#include "../../DX12 Data Structures/DX12PipelineDataStructures.h"
#include "../../DX12RendererConfig.h"
#include "../../DX12 Texture/DX12Texture.h"
#include "../IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12MirrorRenderPipelineInitArgs :
		public DX12PipelinePassInitArgs {
		uint32_t NumberOfMaterials;
		uint32_t NumberOfTextures;
		uint32_t AlignedSizeOfPerMaterialCb;
		std::array<
			D3D12_GPU_VIRTUAL_ADDRESS,
			DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES
		>& PerMaterialCBAddress;
		// since I don't want to use EngineConfig::MAX_TEXTURES in here 
		// as it's code smell, we'll get the pointer to the first texture
		// and then loop over NumberOfTextures
		// this works because our mTextures inside DX12Renderer is a flat array
		const DX12Texture* TexturesData;
	};

	struct DX12MirrorRenderPipelinePerItemPerSubMeshExecuteArgs {
		uint32_t ID;
		uint32_t IndexCount;
		uint32_t StartIndexLocation;
		uint32_t BaseVertexLocation;
	};

	struct DX12MirrorRenderPipelinePerItemExecuteArgs {
		uint32_t ID;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		uint8_t SubMeshCount;
		std::vector<
			DX12MirrorRenderPipelinePerItemPerSubMeshExecuteArgs
		> SubMeshExecuteArgs;
	};

	struct DX12MirrorRenderPipelineExecuteArgs : 
		public DX12PipelinePassExecuteArgs {

		ID3D12DescriptorHeap* DescriptorHeap;
		DX12MirrorRenderPipelinePerItemExecuteArgs* PipelineItemsExecuteArgs;
		uint32_t NumberOfItems;
		uint8_t NumberOfSubMeshesPerItem;
		D3D12_GPU_VIRTUAL_ADDRESS PerPassCBResourceAddress;
		uint32_t AlignedSizeOfPerRenderItemCb;
		D3D12_GPU_VIRTUAL_ADDRESS PerRenderItemCBResourceAddress;
		uint32_t AlignedSizeOfPerRenderItemSubMeshCb;
		D3D12_GPU_VIRTUAL_ADDRESS PerRenderItemSubMeshCBResourceAddress;
		uint32_t NumberOfMaterials;
		CD3DX12_GPU_DESCRIPTOR_HANDLE MaterialsCbvHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE TexturesCbvHandle;
	};
}