#include "DX12PipelinePassAggregator.h"

#include <cassert>

namespace Engine::EngineRenderer::DX12Renderer {
	void DX12PipelinePassAggregator::InsertPass(IDX12PipelinePass* pass) {
		mActiveFrameQueue.push_back(pass);
	}

	void DX12PipelinePassAggregator::Aggregate(
		ID3D12GraphicsCommandList* mainCommandList, 
		DX12PipelinePassAggregatorResult& result
	) {
		// add the main command list to the active list
		result.ActiveLists.push_back(mainCommandList);

		if (mActiveFrameQueue.size() > 0) {
			// close the main command list
			mainCommandList->Close();

			// use the last command list in the active frames queue to perform the 
			// resource transition from render_target to present
			for (int i = 0; i < mActiveFrameQueue.size(); ++i) {
				// if this is the final one, get a handle on it so we can 
				// transition, but don't close it yet
				if (i == (mActiveFrameQueue.size() - 1)) {
					result.ActiveLists.push_back(mActiveFrameQueue.back()->GetCommandList());
				}
				// close all but the last, so (0...n-2) command lists
				else {
					ID3D12GraphicsCommandList* cList = (ID3D12GraphicsCommandList*)mActiveFrameQueue[i]->GetCommandList();
					HRESULT hr = cList->Close();
					if (FAILED(hr)) {
						Logger::ERR(L"Closing the command list has failed. This should NOT happeen");
						ThrowDWException(hr);
					}
					result.ActiveLists.push_back(cList);
				}
			}

		}
	}

	void DX12PipelinePassAggregator::Reset() {
		mActiveFrameQueue.clear();
	}
}