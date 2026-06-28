#pragma once

#include <array>
#include <cstdint>
#include "../../DX12 Texture/DX12Texture.h"
#include "../../DX12RendererConfig.h"
#include "../IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12DebugSystemPipelinePassInitArgs : public DX12PipelinePassInitArgs {
		uint32_t DebugSystemMaxCharacters;
		uint32_t FontAtlasIndex;
		DX12Texture& FontAtlasTexture;
		std::array<ID3D12Resource*, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES> CBResources;
	};

	struct DX12DebugSystemPipelinePassExecuteArgs : public DX12PipelinePassExecuteArgs {
		ID3D12Resource* Resource;
		uint32_t NumberOfCharacters;
	};

	class DX12DebugSystemPipelinePass : public IDX12PipelinePass {
	public:
		~DX12DebugSystemPipelinePass() = default;

	protected:
		bool OnInitialize(
			const DX12PipelinePassInitArgs& args
		) override;
		void OnShutdown() override;
		void OnExecute(const DX12PipelinePassExecuteArgs& args) override;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDescriptorHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mVsByteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mPsByteCode = nullptr;

	private:
		void Execute(
			const DX12DebugSystemPipelinePassExecuteArgs& args
		);
		bool CreateConstantBufferDescriptors(
			const DX12DebugSystemPipelinePassInitArgs& args
		);
		bool CreateConstantBufferViews(
			const DX12DebugSystemPipelinePassInitArgs& args
		);
		bool CreateRootSignature(
			const DX12DebugSystemPipelinePassInitArgs& args
		);
		bool CreateShaders();
		bool CreatePipelineStateObject(
			const DX12DebugSystemPipelinePassInitArgs& args
		);
	};
}