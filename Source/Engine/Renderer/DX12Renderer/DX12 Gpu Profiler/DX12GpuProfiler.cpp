#include "DX12GpuProfiler.h"

#include "../d3dx12.h"
#include "../../../../Helper/Logger.h"

namespace Engine::EngineRenderer::DX12Renderer {

	DX12GpuProfiler::DX12GpuProfiler(uint8_t maxPasses) :
        mMappedReadbackData(nullptr),
        mGpuFrequency(0),
        mOneOverGpuFrequency(0),
        mCurrentFrameIndex(0),
		mMaxPasses(maxPasses)
	{}

	bool DX12GpuProfiler::Initialize(
		ID3D12Device* device, 
		ID3D12CommandQueue* commandQueue
	) {

		if (device == nullptr || 
			commandQueue == nullptr) {
			Logger::ERR(L"Device or command queue are nullptr, cannot Initialize Gpu Profiler!");
			return false; 
		}

        // initially all indexes point to -1 which means
        // no pass has run yet
        for (auto& data : mFrameProfilerData) {
            std::fill(
                data.PassIndexes.begin(),
                data.PassIndexes.end(), 
                INVALID_PASS_INDEX
            );
            data.TotalTimestampsRecorded = 0;
        }

        // get the gpu timestamp frequency
        ThrowIfFailed(commandQueue->GetTimestampFrequency(&mGpuFrequency));

        if (mGpuFrequency == 0) {
            Logger::ERR(L"Failed to get GPU timestamp frequency. Profiler disabled.");
            return false;
        }
        else {
            mOneOverGpuFrequency = 1.0 / (double)mGpuFrequency;
        }

        // 2. Create Query Heap
        D3D12_QUERY_HEAP_DESC heapDesc = {};
        
        // Trade-off: We COULD size the heap to just (2 * mMaxQueriesPerFrame).
        // However, that would require us to issue a ResolveQueryData command 
        // immediately after every pipeline pass to prevent the next frame from 
        // overwriting the data. For 10 passes, that means 10 separate GPU memory 
        // copy operations scattered throughout the command list.
        //
        // Instead, we size the heap to (NUMBER_OF_FRAME_RESOURCES * mMaxQueriesPerFrame).
        // This gives us enough unique slots to hold all query data for the entire frame, 
        // allowing us to batch them into a single ResolveQueryData command at the 
        // very end of the frame, which is much better for GPU performance.
        // * 2 because we want to write the tick at start of a pass, and end of a pass
        // so we can compute time for each pass
        uint8_t totalTimestamps = mMaxPasses * mTimestampsPerPass;
        heapDesc.Count = DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES * totalTimestamps;
        heapDesc.NodeMask = 0;
        heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

        ThrowIfFailed(device->CreateQueryHeap(
            &heapDesc, 
            IID_PPV_ARGS(&mQueryHeap)
        ));
        mQueryHeap->SetName(L"GPU Profiler Query Heap");

        // 3. Create Readback Buffer
        UINT64 bufferSize = heapDesc.Count * sizeof(UINT64);
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        ThrowIfFailed(device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            // the gpu copies the data to this resource during ResolveQuery
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&mReadbackBuffer)
        ));

        mReadbackBuffer->SetName(L"Gpu Profiler Readback Buffer.");

        // 4. Map the Readback Buffer so the CPU can read it
        CD3DX12_RANGE readRange(0, bufferSize); // CPU will not write to this buffer
        ThrowIfFailed(mReadbackBuffer->Map(
            0, 
            &readRange,
            reinterpret_cast<void**>(&mMappedReadbackData)
        ));

        return true;
	}

    void DX12GpuProfiler::ShutDown() {
        if (mReadbackBuffer && mMappedReadbackData) {
            mReadbackBuffer->Unmap(0, nullptr);
            mMappedReadbackData = nullptr;
        }

        mReadbackBuffer.Reset();
        mQueryHeap.Reset();
    }

    void DX12GpuProfiler::BeginFrame() {
        ResetPassProfileDataForCurrentFrame();
    }

    void DX12GpuProfiler::BeginPass(
        ID3D12GraphicsCommandList* cmdList, 
        uint8_t passIndex
    ) {
        uint8_t timestampsRecorded = mFrameProfilerData[mCurrentFrameIndex].TotalTimestampsRecorded;

        if (passIndex >= DX12RendererConfig::MAX_NUMBER_OF_PASSES ||
            (timestampsRecorded + mTimestampsPerPass) >= (mMaxPasses * mTimestampsPerPass)) { return; }

        UINT queryIndex =
            (mCurrentFrameIndex * mMaxPasses * mTimestampsPerPass) +
            timestampsRecorded;

        mFrameProfilerData[mCurrentFrameIndex].PassIndexes[(uint8_t)passIndex] = timestampsRecorded;
        mFrameProfilerData[mCurrentFrameIndex].TotalTimestampsRecorded += 1;

        //The actual hardware clock 
        // is read and written when we call EndQuery().
        cmdList->EndQuery(
            mQueryHeap.Get(),
            D3D12_QUERY_TYPE_TIMESTAMP, 
            queryIndex
        );
    }

    void DX12GpuProfiler::EndPass(ID3D12GraphicsCommandList* cmdList, uint8_t passIndex) {
        // +1 because we write ticks in pairs
        // ex: for pass index 0, we write in index 0 and 1
        // then compute the difference in resolvequery

        // here we don't check if queryIndex will overflow because 
        // we've already check in begin pass if we'll be able to write
        // both, before and after
        uint8_t timestampsRecorded = mFrameProfilerData[mCurrentFrameIndex].TotalTimestampsRecorded;

        UINT queryIndex =
            (mCurrentFrameIndex * mMaxPasses * mTimestampsPerPass) +
            timestampsRecorded;

        mFrameProfilerData[mCurrentFrameIndex].TotalTimestampsRecorded += 1;
  
        cmdList->EndQuery(
            mQueryHeap.Get(),
            D3D12_QUERY_TYPE_TIMESTAMP,
            queryIndex
        );
    }

    void DX12GpuProfiler::ResolveAllQueries(ID3D12GraphicsCommandList* cmdList) {
        if (mFrameProfilerData[mCurrentFrameIndex].TotalTimestampsRecorded == 0) { return; }

        UINT startIndex = mCurrentFrameIndex * mMaxPasses * mTimestampsPerPass;
        UINT64 destinationOffset = startIndex * sizeof(UINT64);

        cmdList->ResolveQueryData(
            mQueryHeap.Get(),
            D3D12_QUERY_TYPE_TIMESTAMP,
            startIndex,
            mFrameProfilerData[mCurrentFrameIndex].TotalTimestampsRecorded, // numQueries
            mReadbackBuffer.Get(),
            destinationOffset
        );
    }

    void DX12GpuProfiler::SetFrameIndex(uint8_t frameIndex) {
       mCurrentFrameIndex = frameIndex; 
    }

    const DX12GpuProfilerResults& DX12GpuProfiler::GetProfilerResults() {
        UINT readFrameIndex = (mCurrentFrameIndex * mMaxPasses * mTimestampsPerPass);

        for (uint8_t i = 0; i < DX12RendererConfig::MAX_NUMBER_OF_PASSES; ++i) {
            int8_t passStartIndex = mFrameProfilerData[mCurrentFrameIndex].PassIndexes[i];

            if (passStartIndex == INVALID_PASS_INDEX) {
                mProfilerResults.PassTimes[i] = 0.f;
                continue;
            }

            UINT startIndex = readFrameIndex + passStartIndex;
            UINT endIndex = startIndex + 1;

            // Read the two distinct timestamps from the mapped readback memory
            UINT64 startTick = mMappedReadbackData[startIndex];
            UINT64 endTick = mMappedReadbackData[endIndex];

            // Calculate delta and convert to milliseconds
            UINT64 deltaTicks = endTick - startTick;
            double timeSeconds = static_cast<double>(deltaTicks) * mOneOverGpuFrequency;
            mProfilerResults.PassTimes[i] = (float)timeSeconds * 1000.f;
        }
        return mProfilerResults;
    }

#pragma region Private

    void DX12GpuProfiler::ResetPassProfileDataForCurrentFrame() { 
        auto& currentData = mFrameProfilerData[mCurrentFrameIndex];
        std::fill(
            currentData.PassIndexes.begin(),
            currentData.PassIndexes.end(), 
            INVALID_PASS_INDEX
        );
        currentData.TotalTimestampsRecorded = 0;
    }

#pragma endregion
}