#pragma once

#include <array>
#include <cstdint>
#include "../../DX12 Data Structures/DX12PipelineDataStructures.h"
#include "../../DX12RendererConfig.h"
#include "../../DX12 Texture/DX12Texture.h"
#include "../IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12OpaqueRenderPipelineInitArgs : public DX12PipelinePassInitArgs {
		uint32_t NumberOfMaterials;
		uint32_t NumberOfTextures;
		uint32_t AlignedSizeOfPerMaterialCb;
		std::array<D3D12_GPU_VIRTUAL_ADDRESS, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES>& PerMaterialCBAddress;
		// since I don't want to use EngineConfig::MAX_TEXTURES in here 
		// as it's code smell, we'll get the pointer to the first texture
		// and then loop over NumberOfTextures
		// this works because our mTextures inside DX12Renderer is a flat array
		const DX12Texture* TexturesData;
	};

	struct DX12OpaqueRenderPipelinePerItemPerSubMeshArgs {
		uint32_t ID;
		uint32_t IndexCount;
		uint32_t StartIndexLocation;
		uint32_t BaseVertexLocation;
	};

	struct DX12OpaqueRenderPipelinePerItemExecuteArgs {
		uint32_t ID;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		uint8_t SubMeshCount;
		std::vector< DX12OpaqueRenderPipelinePerItemPerSubMeshArgs> SubMeshExecuteArgs;
	};

	struct DX12OpaqueRenderPipelineExecuteArgs : public DX12PipelinePassExecuteArgs {
		ID3D12DescriptorHeap* DescriptorHeap;
		DX12OpaqueRenderPipelinePerItemExecuteArgs* PipelineItemsExecuteArgs;
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

	class DX12OpaqueRenderPipelinePass : public IDX12PipelinePass {
	public:
		~DX12OpaqueRenderPipelinePass() = default;

	protected:
		bool OnInitialize(
			const DX12PipelinePassInitArgs& args
		) override;
		void OnShutdown() override;
		void OnExecute(const DX12PipelinePassExecuteArgs& args) override;

	private:
		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
		Microsoft::WRL::ComPtr<ID3DBlob> mVsByteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mPsByteCode = nullptr;
		uint32_t mTexturesCbHeapOffset = 0;
		uint8_t mPerObjectCBIndex = 0;
		uint8_t mPerObjectPerSubMeshCBIndex = 1;
		uint8_t mObjectsPerPassCBIndex = 2;
		uint8_t mMaterialsCBIndex = 3;
		uint8_t mTexturesCBIndex = 4;

	private:
		void Execute(
			const DX12OpaqueRenderPipelineExecuteArgs& args
		);
		bool CreateRootSignature(
			const DX12OpaqueRenderPipelineInitArgs& args
		);
		bool CreateShadersAndInputLayout();
		bool CreatePipelineStateObject(
			const DX12OpaqueRenderPipelineInitArgs& args
		);
	};
}