#include "IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	bool IDX12PipelinePass::Initialize(const PipelinePassInitArgs& args) {
		if (!OnInitialize(args)) { return false; }

		if (!InitializeCommandList(args.Device, args.Allocator)) { return false; }

		return true;
	}

	ID3D12CommandList* IDX12PipelinePass::GetCommandList() { return mCommandList.Get(); }

	bool IDX12PipelinePass::InitializeCommandList(
		ID3D12Device* device,
		ID3D12CommandAllocator* commandAllocator
	) {
		if (mPipelineStateObject == nullptr) { return false; }

		ThrowIfFailed(
			device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				commandAllocator,			// Associated command allocator
				mPipelineStateObject.Get(),	// Associated PipelineStateObject
				IID_PPV_ARGS(mCommandList.GetAddressOf())
			)
		);

		return true;
	}
}