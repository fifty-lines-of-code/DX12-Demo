#pragma once

#include <d3d12.h>
#include "../../../../Helper/Helper.h"
#include <wrl.h>

namespace Engine::EngineRenderer::DX12Renderer {

	struct PipelinePassInitArgs {
		ID3D12Device* Device;
		ID3D12CommandAllocator* Allocator;
		DXGI_FORMAT BackBufferFormat;
		DXGI_FORMAT DepthStencilFormat;
	};

	class IDX12PipelinePass {
	public:
		IDX12PipelinePass() = default;
		virtual ~IDX12PipelinePass() = default;

		bool Initialize(const PipelinePassInitArgs& args);
		virtual bool OnInitialize(const PipelinePassInitArgs& args) = 0;

		ID3D12CommandList* GetCommandList();

	protected:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineStateObject = nullptr;

	private:
		bool InitializeCommandList(
			ID3D12Device* device,
			ID3D12CommandAllocator* commandAllocator
		);
	};
}