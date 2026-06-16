#pragma once

#include <d3d12.h>
#include "../DX12RendererConfig.h"
#include "../../../../Helper/Helper.h"
#include "../../../../Helper/Logger.h"
#include <wrl.h>

namespace Engine::EngineRenderer::DX12Renderer {

	struct PipelinePassInitArgs {
		ID3D12Device* Device;
		ID3D12CommandAllocator* Allocator;
		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthStencilFormat;
	};

	struct IPipelinePassExecuteArgs {
		uint32_t CurrentFrameIndex;
		UINT CbvSrvUavDescriptorSize;
		ID3D12Resource* CurrentBackBufferResource;
		D3D12_CPU_DESCRIPTOR_HANDLE& BackBufferView;
		D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilView;
		D3D12_VIEWPORT& Viewport;
		D3D12_RECT& ScissorRect;
	};

	class IDX12PipelinePass {
	public:
		IDX12PipelinePass();
		virtual ~IDX12PipelinePass() = default;

		bool Initialize(const PipelinePassInitArgs& args);
		virtual void ShutDown();

		ID3D12CommandList* GetCommandList() noexcept;
		bool GetIsInitialized() const noexcept;

	protected:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocators[DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineStateObject = nullptr;
		bool mIsInitialized;

	private:
		virtual bool OnInitialize(const PipelinePassInitArgs& args) = 0;
		bool InitializeCommandAllocators(const PipelinePassInitArgs& args);
		bool InitializeCommandList(
			ID3D12Device* device,
			ID3D12CommandAllocator* commandAllocator
		);
	};
}