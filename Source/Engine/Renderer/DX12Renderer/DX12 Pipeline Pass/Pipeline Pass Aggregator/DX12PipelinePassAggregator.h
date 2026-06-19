#pragma once

#include "../IDX12PipelinePass.h"
#include <vector>

namespace Engine::EngineRenderer::DX12Renderer {

	struct DX12PipelinePassAggregatorResult {
		std::vector<ID3D12CommandList*> ActiveLists;
	};

	class DX12PipelinePassAggregator {
	public:
		DX12PipelinePassAggregator() = default;
		~DX12PipelinePassAggregator() = default;

		void InsertPass(IDX12PipelinePass* pass);
		void Aggregate(
			ID3D12GraphicsCommandList* commandList,
			DX12PipelinePassAggregatorResult& result
		);
		void Reset();

	private:
		std::vector<IDX12PipelinePass*> mActiveFrameQueue;
	};
}