#pragma once

#include "../d3dx12.h"
#include "../DX12 Gpu Profiler/DX12GpuProfiler.h"
#include "../DX12RendererConfig.h"
#include "../../../../Helper/Helper.h"
#include "../../../../Helper/Logger.h"
#include <wrl.h>

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12PipelinePassInitArgs {
		ID3D12Device* Device;
		ID3D12CommandAllocator* Allocator;
		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthStencilFormat;
		UINT CbvSrvUavDescriptorSize;
	};

	struct DX12PipelinePassExecuteArgs {
		uint32_t CurrentFrameIndex;
		uint32_t CurrentBackBufferIndex;
		UINT CbvSrvUavDescriptorSize;
		ID3D12Resource* CurrentBackBufferResource;
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferView;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView;
		D3D12_VIEWPORT& Viewport;
		D3D12_RECT& ScissorRect;
		DX12GpuProfiler& GpuProfiler;
	};

	class IDX12PipelinePass {
	public:
		IDX12PipelinePass();
		~IDX12PipelinePass() = default;

		// delete copy and assignment operators
		// so that we don't have any unwanted copies
		// that perform shallow copies and maybe 
		// go out of scope at unexpected times
		IDX12PipelinePass(const IDX12PipelinePass&) = delete;
		IDX12PipelinePass& operator=(const IDX12PipelinePass&) = delete;
		IDX12PipelinePass(IDX12PipelinePass&&) = delete;
		IDX12PipelinePass& operator=(IDX12PipelinePass&&) = delete;

		bool Initialize(const DX12PipelinePassInitArgs& args);
		void ShutDown();

		void ExecutePass(const DX12PipelinePassExecuteArgs& args);
		ID3D12CommandList* GetCommandList() noexcept;
		bool GetIsInitialized() const noexcept;

	protected:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocators[DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES];
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineStateObject = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		bool mIsInitialized;

	protected:
		virtual bool OnInitialize(const DX12PipelinePassInitArgs& args) = 0;
		virtual void OnShutdown() = 0;
		virtual void OnExecute(const DX12PipelinePassExecuteArgs& args) = 0;

	private:
		bool InitializeCommandAllocators(const DX12PipelinePassInitArgs& args);
		bool InitializeCommandList(
			ID3D12Device* device,
			ID3D12CommandAllocator* commandAllocator
		);
	};
}