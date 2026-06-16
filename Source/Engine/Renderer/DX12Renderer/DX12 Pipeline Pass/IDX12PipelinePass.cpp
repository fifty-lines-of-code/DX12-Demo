#include "IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	IDX12PipelinePass::IDX12PipelinePass() : 
		mIsInitialized(false)
	{}

	bool IDX12PipelinePass::Initialize(const PipelinePassInitArgs& args) {
		if (!OnInitialize(args)) { return false; }

		if (mPipelineStateObject == nullptr) {
			Logger::ERR(L"Pipeline State Object should be Initialized");
			return false;
		}

		if (!InitializeCommandAllocators(args)) { return false; }
		if (!InitializeCommandList(args.Device, args.Allocator)) { return false; }

		mIsInitialized = true;

		return true;
	}

	void IDX12PipelinePass::ShutDown() {
		if (mPipelineStateObject != nullptr) { mPipelineStateObject.Reset(); }
		if (mCommandList != nullptr) { mCommandList.Reset(); }
		for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++i) {
			if (mCommandAllocators[i] != nullptr) {
				mCommandAllocators[i].Reset();
			}
		}
	}

	ID3D12CommandList* IDX12PipelinePass::GetCommandList() noexcept {
		return mCommandList.Get();
	}

	bool IDX12PipelinePass::GetIsInitialized() const noexcept { return mIsInitialized; }

#pragma region Private

	bool IDX12PipelinePass::InitializeCommandAllocators(const PipelinePassInitArgs& args) {
		for (int i = 0; i < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++i) {
			args.Device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT, 
				IID_PPV_ARGS(mCommandAllocators[i].GetAddressOf())
			);
			mCommandAllocators[i]->SetName(
				(L"Pass Command Allocator: " + std::to_wstring(i)).c_str()
			);
		}

		return true;
	}

	bool IDX12PipelinePass::InitializeCommandList(
		ID3D12Device* device,
		ID3D12CommandAllocator* commandAllocator
	) {
		ThrowIfFailed(
			device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				mCommandAllocators[0].Get(),			// Associated command allocator
				mPipelineStateObject.Get(),	// Associated PipelineStateObject
				IID_PPV_ARGS(mCommandList.GetAddressOf())
			)
		);
		mCommandList->SetName(L"Pipeline Pass Command List");

		mCommandList->Close();

		return true;
	}
#pragma endregion
}