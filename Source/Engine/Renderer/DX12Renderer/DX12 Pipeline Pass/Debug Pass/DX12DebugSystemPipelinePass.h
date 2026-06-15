#pragma once

#include <cstdint>
#include "../../DX12 Texture/DX12Texture.h"
#include "../IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DebugSystemPipelinePassInitArgs : public PipelinePassInitArgs {
		uint32_t NoFrameResources;
		uint32_t DebugSystemMaxCharacters;
		uint32_t FontAtlasIndex;
		UINT CbvSrvUavDescriptorSize;
		DX12Texture& FontAtlasTexture;
		ID3D12Resource* CBResources[3];
	};

	class DX12DebugSystemPipelinePass : public IDX12PipelinePass {
	public:
		DX12DebugSystemPipelinePass() = default;
		~DX12DebugSystemPipelinePass() = default;

		bool OnInitialize(
			const DebugSystemPipelinePassInitArgs& args
		);

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDescriptorHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mVsByteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mPsByteCode = nullptr;

	private:
		bool CreateConstantBufferDescriptors(
			const DebugSystemPipelinePassInitArgs& args
		);

		bool CreateConstantBufferViews(
			const DebugSystemPipelinePassInitArgs& args
		);

		bool CreateDebugRootSignature(
			const DebugSystemPipelinePassInitArgs& args
		);

		bool CreateShadersAndInputLayout();
		bool CreateDebugPipelineStateObject(
			const DebugSystemPipelinePassInitArgs& args
		);
	};
}