#pragma once

#include <array>
#include <cstdint>
#include "../../DX12RendererConfig.h"
#include "../../DX12 Data Structures/DX12ResourceDataStructures.h"
#include <dxgi1_4.h>
#include "../IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12BlurPipelinePassInitArgs : public DX12PipelinePassInitArgs {
		uint32_t SwapchainBufferCount;
		std::array<ID3D12Resource*, DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS> SwapChainBuffers;
		uint32_t WindowWidth;
		uint32_t WindowHeight;
	};

	struct DX12BlurPipelinePassExecuteArgs : public DX12PipelinePassExecuteArgs {
		uint32_t WindowWidth;
		uint32_t WindowHeight;
	};

	class DX12BlurPipelinePass : public IDX12PipelinePass {
	public:
		DX12BlurPipelinePass();
		~DX12BlurPipelinePass() = default;

		void OnResize(
			uint32_t width, 
			uint32_t height,
			const DX12BlurPipelinePassInitArgs& args
		);

	protected:
		bool OnInitialize(
			const DX12PipelinePassInitArgs& args
		) override;
		void OnShutdown() override;
		void OnExecute(const DX12PipelinePassExecuteArgs& args) override;

	private:
		DX12BlurComputeConstants mComputeConstants;
		Microsoft::WRL::ComPtr<ID3D12Resource> mScratchTextureResource = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> mCsByteCode = nullptr;
		
	private:
		void Execute(const DX12BlurPipelinePassExecuteArgs& args);
		bool CreateDescriptorHeap(
			const DX12BlurPipelinePassInitArgs& args
		);
		bool CreateDescriptorViews(
			const DX12BlurPipelinePassInitArgs& args
		);
		bool CreateRootSignature(
			const DX12BlurPipelinePassInitArgs& args
		);
		bool CreateShaders();
		bool CreatePipelineStateObject(
			const DX12BlurPipelinePassInitArgs& args
		);
		void CreateScratchTexture(
			const DX12BlurPipelinePassInitArgs& args
		);
	};
}