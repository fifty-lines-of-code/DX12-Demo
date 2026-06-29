#pragma once

#include "DX12MirrorRenderPipelinePassDataStructures.h"
#include "../IDX12PipelinePass.h"

namespace Engine::EngineRenderer::DX12Renderer {

	class DX12MirrorRenderPipelinePass : 
		public IDX12PipelinePass {
	public:
		DX12MirrorRenderPipelinePass() = default;
		~DX12MirrorRenderPipelinePass() = default;

		DX12MirrorRenderPipelinePass(const DX12MirrorRenderPipelinePass&) = delete;
		DX12MirrorRenderPipelinePass& operator=(const DX12MirrorRenderPipelinePass&) = delete;
		DX12MirrorRenderPipelinePass(DX12MirrorRenderPipelinePass&&) = delete;
		DX12MirrorRenderPipelinePass& operator=(DX12MirrorRenderPipelinePass&&) = delete;

	protected:
		bool OnInitialize(
			const DX12PipelinePassInitArgs& args
		) override;
		void OnShutdown() override;
		void OnExecute(
			const DX12PipelinePassExecuteArgs& args
		) override;

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
			const DX12MirrorRenderPipelineExecuteArgs& args
		);
		bool CreateRootSignature(
			const DX12MirrorRenderPipelineInitArgs& args
		);
		bool CreateShadersAndInputLayout();
		bool CreatePipelineStateObject(
			const DX12MirrorRenderPipelineInitArgs& args
		);
	};
}