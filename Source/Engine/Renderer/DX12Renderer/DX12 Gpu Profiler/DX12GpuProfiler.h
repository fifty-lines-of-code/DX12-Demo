#pragma once

#include <array>
#include <cstdint>
#include <d3d12.h>
#include "../DX12RendererConfig.h"
#include <wrl.h>

namespace Engine::EngineRenderer::DX12Renderer {

	class DX12GpuProfiler {
	public:
		DX12GpuProfiler(uint8_t maxPasses);
		~DX12GpuProfiler() = default;

		// delete copy and assignment 
		DX12GpuProfiler(const DX12GpuProfiler&) = delete;
		DX12GpuProfiler& operator=(const DX12GpuProfiler&) = delete;

		bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue);
		void ShutDown();

		void BeginFrame();
		void BeginPass(ID3D12GraphicsCommandList* cmdList, uint8_t passIndex);
		void EndPass(ID3D12GraphicsCommandList* cmdList, uint8_t passIndex);

		void ResolveAllQueries(ID3D12GraphicsCommandList* cmdList);

		void SetFrameIndex(uint8_t frameIndex);
		float GetElapsedTimeMilliseconds(uint8_t passIndex);

	private:
		static constexpr int INVALID_PASS_INDEX = -1;

		// we save the index within the heap for each pass that's run
		// based on the counter var
		// this way we can execute any pass in any order and read their
		// profiling data by indexing into the result for each pass
		std::array<int, DX12RendererConfig::MAX_NUMBER_OF_PASSES> mPassIndexes;

		// dx12 resources
		Microsoft::WRL::ComPtr<ID3D12QueryHeap> mQueryHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource>  mReadbackBuffer;
		// Mapped CPU pointer for the readback buffer
		UINT64* mMappedReadbackData;
		UINT64 mGpuFrequency;
		double mOneOverGpuFrequency;
		uint8_t mFrameIndex;
		uint8_t mMaxPasses;
		// * 2 because we want to write the tick at start of a pass, 
		// and end of a pass so we can compute time for each pass
		const uint8_t mTimestampsPerPass = 2;
		uint8_t mPassCounter;

	private:
		void ResetPassIndexes();
	};
}