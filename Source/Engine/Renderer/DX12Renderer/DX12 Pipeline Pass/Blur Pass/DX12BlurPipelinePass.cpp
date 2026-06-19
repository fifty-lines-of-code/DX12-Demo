#include "DX12BlurPipelinePass.h"

#include "../../d3dx12.h"
#include "../../DX12RendererHelper.h"
#include "../../DX12 Data Structures/DX12ResourceDataStructures.h"
#include "../../../../../Helper/Helper.h"
#include "../../../../../Helper/Logger.h"
#include <windows.h>

namespace Engine::EngineRenderer::DX12Renderer {

	DX12BlurPipelinePass::DX12BlurPipelinePass() {
		// set radius intensity
		// todo: move this out to somewhere else
		float blurRadius = 10.f;
		mComputeConstants.BlurRadius = (int)blurRadius;
		// Standard deviation controls the spread
		float sigma = blurRadius / 1.5f;
		float twoSigmaSq = 2.0f * sigma * sigma;
		float oneOverTwoSigmaSq = 1.0f / twoSigmaSq;
		mComputeConstants.OneOverTwoSigmaSq = oneOverTwoSigmaSq;
	}

	void DX12BlurPipelinePass::OnResize(
		uint32_t width,
		uint32_t height, 
		const DX12BlurPipelinePassInitArgs& args
	) {
		mScratchTextureResource.Reset();
		CreateScratchTexture(args);
		CreateDescriptorViews(args);
	}

#pragma region Private

	bool DX12BlurPipelinePass::OnInitialize(
		const DX12PipelinePassInitArgs& args
	) {
		const DX12BlurPipelinePassInitArgs& debugArgs = static_cast<const DX12BlurPipelinePassInitArgs&>(args);

		if (!CreateDescriptorHeap(debugArgs)) { return false; }
		if (!CreateRootSignature(debugArgs)) { return false; }
		if (!CreateShaders()) { return false; }
		if (!CreatePipelineStateObject(debugArgs)) { return false; }

		return true;
	}

	void DX12BlurPipelinePass::OnShutdown() {
		if (mScratchTextureResource != nullptr) { mScratchTextureResource.Reset(); }
		if (mCsByteCode != nullptr) { mCsByteCode.Reset(); }
	}

	void DX12BlurPipelinePass::OnExecute(const DX12PipelinePassExecuteArgs& args) {
		const DX12BlurPipelinePassExecuteArgs& bArgs = static_cast<const DX12BlurPipelinePassExecuteArgs&>(args);
		Execute(bArgs);
	}

	void DX12BlurPipelinePass::Execute(
		const DX12BlurPipelinePassExecuteArgs& args
	) {
		ID3D12CommandAllocator* allocator = mCommandAllocators[args.CurrentFrameIndex].Get();
		ThrowIfFailed(allocator->Reset());

		// reset the command list
		ThrowIfFailed(mCommandList->Reset(allocator, mPipelineStateObject.Get()));
		mCommandList->SetComputeRootSignature(mRootSignature.Get());

		// Bind the Blur Descriptor Heap
		ID3D12DescriptorHeap* heaps[] = { mDescriptorHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

		// Calculate the exact Dispatch thread grid counts (16x16 pixel blocks)
		UINT groupCountX = static_cast<UINT>(ceil(args.WindowWidth / 16.0f));
		UINT groupCountY = static_cast<UINT>(ceil(args.WindowHeight / 16.0f));

		// blur constants
		// set screen size
		mComputeConstants.ScreenSize = DirectX::XMFLOAT2(
			(float)args.WindowWidth,
			(float)args.WindowHeight
		);;

		// ========================================================================
		// PASS 1: HORIZONTAL BLUR
		// ========================================================================

		D3D12_RESOURCE_BARRIER pass1Barriers[2] = {
			// transition back buffer to non pixel resource so we can read it
			CD3DX12_RESOURCE_BARRIER::Transition(
				args.CurrentBackBufferResource,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			),
				// transition our scratch texture resource to unordered access so we can write to it
				CD3DX12_RESOURCE_BARRIER::Transition(
					mScratchTextureResource.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				) // both the slices
		};
		mCommandList->ResourceBarrier(_countof(pass1Barriers), pass1Barriers);

		// Setup Pass 1 Constant Data
		mComputeConstants.BlurDirection = DirectX::XMFLOAT2(1.0f, 0.0f); // Horizontal step
		mCommandList->SetComputeRoot32BitConstants(0, 8, &mComputeConstants, 0);

		// Bind Descriptor Tables using our heap handles
		CD3DX12_GPU_DESCRIPTOR_HANDLE heapStart(
			mDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);

		// Table 1 (t0): Points to Slot 0 or 1 for back buffer as SRV
		CD3DX12_GPU_DESCRIPTOR_HANDLE backBufferAsSRV(
			heapStart,
			args.CurrentBackBufferIndex,
			args.CbvSrvUavDescriptorSize
		);
		mCommandList->SetComputeRootDescriptorTable(1, backBufferAsSRV);

		// Table 2 (u0): Points to Slot 3 (Scratch Slice 0 as UAV)
		CD3DX12_GPU_DESCRIPTOR_HANDLE scratchUAV(
			heapStart,
			3,
			args.CbvSrvUavDescriptorSize
		);
		mCommandList->SetComputeRootDescriptorTable(2, scratchUAV);

		// Unleash Pass 1 hardware threads
		mCommandList->Dispatch(groupCountX, groupCountY, 1);

		// ========================================================================
		// PASS 2: VERTICAL BLUR 
		// ========================================================================

		// Setup Pass 2 Constant Data
		mComputeConstants.BlurDirection = DirectX::XMFLOAT2(0.0f, 1.0f); // Vertical step
		mCommandList->SetComputeRoot32BitConstants(0, 8, &mComputeConstants, 0);

		// Table 1 (t0): Points to slice 0 of blur scratch descriptor (slot 2) to read it as SRV
		CD3DX12_GPU_DESCRIPTOR_HANDLE scratchAsSRV(
			heapStart,
			2,
			args.CbvSrvUavDescriptorSize
		);
		mCommandList->SetComputeRootDescriptorTable(1, scratchAsSRV);

		// Table 2 (u0): Points to Slot 4 (scratch slice 1 as UAV)
		CD3DX12_GPU_DESCRIPTOR_HANDLE scratchAsUAV(
			heapStart,
			4,
			args.CbvSrvUavDescriptorSize
		);
		mCommandList->SetComputeRootDescriptorTable(2, scratchAsUAV);

		// Pass 2 hardware threads
		mCommandList->Dispatch(groupCountX, groupCountY, 1);

		// ========================================================================
		// After PASS 2: Copy result of VERTICAL BLUR to back buffer
		// ========================================================================

		D3D12_RESOURCE_BARRIER midBarriers[2] = {
			// transition blur scratch resource slice 1 to copy source
			CD3DX12_RESOURCE_BARRIER::Transition(
				mScratchTextureResource.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				1 // slice 1
			),
				// transition the back buffer to copy dest
				CD3DX12_RESOURCE_BARRIER::Transition(
					args.CurrentBackBufferResource,
					D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_COPY_DEST
				)
		};
		mCommandList->ResourceBarrier(_countof(midBarriers), midBarriers);

		// perform the copy from slice 1 of blur scratch to back buffer
		CD3DX12_TEXTURE_COPY_LOCATION destLocation(args.CurrentBackBufferResource, 0);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(
			mScratchTextureResource.Get(), 1
		); // Index 1 = Slice 1

		// actual copy command
		mCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &sourceLocation, nullptr);

		// ========================================================================
		// CLEANUP BARRIER: Return Backbuffer to Render Target state
		// ========================================================================
		// Bring the backbuffer back to its standard state so engine can keep drawing as usual

		D3D12_RESOURCE_BARRIER cleanupBarriers[3] = {
			// transition back buffer to render target
			CD3DX12_RESOURCE_BARRIER::Transition(
				args.CurrentBackBufferResource,
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			),
				// transition slice 0 of blur scratch to common
				CD3DX12_RESOURCE_BARRIER::Transition(
					mScratchTextureResource.Get(),
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_COMMON,
					0
				),
				// transition slice 1 of blur scratch to common
				CD3DX12_RESOURCE_BARRIER::Transition(
					mScratchTextureResource.Get(),
					D3D12_RESOURCE_STATE_COPY_SOURCE,
					D3D12_RESOURCE_STATE_COMMON,
					1
				)
		};
		mCommandList->ResourceBarrier(_countof(cleanupBarriers), cleanupBarriers);

		// do not close the command list here, the aggregator will close it
	}

	bool DX12BlurPipelinePass::CreateDescriptorHeap(
		const DX12BlurPipelinePassInitArgs& args
	) {
		// 3 for our scratch buffer we'll write to and read from
		// 2 for reading the back buffer (2 since we are double buffered)
		UINT numberOfDescriptors = 5;

		// Describe the CBV descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numberOfDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		// Create the CBV descriptor heap
		ThrowIfFailed(
			args.Device->CreateDescriptorHeap(
				&cbvHeapDesc,
				IID_PPV_ARGS(&mDescriptorHeap)
			)
		);

		return CreateDescriptorViews(args);
	}

	bool DX12BlurPipelinePass::CreateDescriptorViews(
		const DX12BlurPipelinePassInitArgs& args
	) {
		// we have 5 descriptors

		// 1. Bake the Backbuffer SRVs into Slots 0 and 1
		D3D12_SHADER_RESOURCE_VIEW_DESC bbSrvDesc = {};
		bbSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		bbSrvDesc.Format = args.BackBufferFormat;
		bbSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		bbSrvDesc.Texture2D.MostDetailedMip = 0;
		bbSrvDesc.Texture2D.MipLevels = 1;

		for (uint32_t i = 0; i < args.SwapchainBufferCount; ++i) {
			CD3DX12_CPU_DESCRIPTOR_HANDLE hBbCpu(
				mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				i,
				args.CbvSrvUavDescriptorSize
			);
			args.Device->CreateShaderResourceView(
				args.SwapChainBuffers[i], 
				&bbSrvDesc, 
				hBbCpu
			);
		}

		// 2. Bake the Texture Array Views into Slots 2, 3, and 4
		// Slot 2: SRV targeting Texture Array Slice 0 (Pass 2 Read source)
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc0 = {};
		srvDesc0.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc0.Format = args.BackBufferFormat;
		srvDesc0.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc0.Texture2DArray.MostDetailedMip = 0;
		srvDesc0.Texture2DArray.MipLevels = 1;
		srvDesc0.Texture2DArray.FirstArraySlice = 0;
		srvDesc0.Texture2DArray.ArraySize = 1;

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSlot2(
			mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			2,
			args.CbvSrvUavDescriptorSize
		);
		args.Device->CreateShaderResourceView(
			mScratchTextureResource.Get(),
			&srvDesc0,
			hCpuSlot2
		);

		// Slot 3: UAV targeting Texture Array Slice 0 (Pass 1 Write target)
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc0 = {};
		uavDesc0.Format = args.BackBufferFormat;
		uavDesc0.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc0.Texture2DArray.MipSlice = 0;
		uavDesc0.Texture2DArray.FirstArraySlice = 0;
		uavDesc0.Texture2DArray.ArraySize = 1;

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSlot3(
			mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			3,
			args.CbvSrvUavDescriptorSize
		);
		args.Device->CreateUnorderedAccessView(
			mScratchTextureResource.Get(),
			nullptr,
			&uavDesc0,
			hCpuSlot3
		);

		// Slot 4: UAV targeting Texture Array Slice 1 (Pass 2 Write target)
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc1 = {};
		uavDesc1.Format = args.BackBufferFormat;
		uavDesc1.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc1.Texture2DArray.MipSlice = 0;
		uavDesc1.Texture2DArray.FirstArraySlice = 1;
		uavDesc1.Texture2DArray.ArraySize = 1;

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSlot4(
			mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			4,
			args.CbvSrvUavDescriptorSize
		);
		args.Device->CreateUnorderedAccessView(
			mScratchTextureResource.Get(),
			nullptr,
			&uavDesc1,
			hCpuSlot4
		);

		return true;
	}

	bool DX12BlurPipelinePass::CreateRootSignature(
		const DX12BlurPipelinePassInitArgs& args
	) {
		// 1. Define the parameters
		// We need 3 parameters total: 1 for Root Constants, 2 for Descriptor Tables
		CD3DX12_ROOT_PARAMETER rootParameters[3];

		// Parameter 0: Root Constants (8 DWORDs = 8 32-bit variables)
		// Maps directly to cbuffer BlurConstants : register(b0);
		// See: DX12BlurComputeConstants
		rootParameters[0].InitAsConstants(8, 0);

		// Parameter 1: Input Texture Table (1 SRV)
		// Maps to Texture2D gInputTexture : register(t0);
		CD3DX12_DESCRIPTOR_RANGE srvRange;
		srvRange.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,
			0,
			0
		);
		rootParameters[1].InitAsDescriptorTable(1, &srvRange);

		// Parameter 2: Output Texture Table (1 UAV)
		// Maps to RWTexture2D gOutputTexture : register(u0);
		CD3DX12_DESCRIPTOR_RANGE uavRange;
		uavRange.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			1,
			0,
			0
		);
		rootParameters[2].InitAsDescriptorTable(1, &uavRange);

		// NOTE: we do not use any samplers here as we will be reading the (rgba) value directly
		// from the 2D Texture using thread.xy
		// Maybe we still need samplers? I am not sure.

		// 3. Serialize and Create the Root Signature
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			_countof(rootParameters),
			rootParameters,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE
		);

		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			return false;
		}

		hr = args.Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)
		);
		ThrowIfFailed(hr);

		return true;
	}

	bool DX12BlurPipelinePass::CreateShaders() {
		mCsByteCode = DX12RendererHelper::CompileShader(
			L"Source\\Resources\\Shaders\\Blur\\blur_cs.hlsl",
			nullptr,
			"CS_Main",
			"cs_5_1"
		);

		return true;
	}

	bool DX12BlurPipelinePass::CreatePipelineStateObject(
		const DX12BlurPipelinePassInitArgs& args
	) {
		D3D12_COMPUTE_PIPELINE_STATE_DESC blurPsoDesc = {};

		blurPsoDesc.pRootSignature = mRootSignature.Get();
		blurPsoDesc.CS =
		{
			reinterpret_cast<BYTE*>(mCsByteCode->GetBufferPointer()),
			mCsByteCode->GetBufferSize()
		};
		blurPsoDesc.NodeMask = 0;
		blurPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		blurPsoDesc.CachedPSO.pCachedBlob = nullptr;
		blurPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;

		HRESULT hr = args.Device->CreateComputePipelineState(
			&blurPsoDesc,
			IID_PPV_ARGS(&mPipelineStateObject)
		);
		ThrowIfFailed(hr);

		return true;
	}

	void DX12BlurPipelinePass::CreateScratchTexture(const DX12BlurPipelinePassInitArgs& args) {
		D3D12_RESOURCE_DESC scratchDesc = {};
		scratchDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		scratchDesc.Alignment = 0;
		scratchDesc.Width = args.WindowWidth;
		scratchDesc.Height = args.WindowHeight;
		// size of 2 because we want to write to slice 0 in Pass 1, and
		// read from slice 0 and write to slice 1 in pass 2
		scratchDesc.DepthOrArraySize = 2;
		// Post-processing targets do not use mips
		scratchDesc.MipLevels = 1;
		scratchDesc.Format = args.BackBufferFormat;
		scratchDesc.SampleDesc.Count = 1;
		scratchDesc.SampleDesc.Quality = 0;
		scratchDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		// We need UAV access to this one as we'll write to it in Pass 1 (Horizontal)
		// Pass 2 will read it as a SRV
		scratchDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

		HRESULT hr = args.Device->CreateCommittedResource(
			&defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&scratchDesc,
			D3D12_RESOURCE_STATE_COMMON, // Standard starting resource state
			nullptr,
			IID_PPV_ARGS(&mScratchTextureResource)
		);
		ThrowIfFailed(hr);

		mScratchTextureResource.Get()->SetName(L"Blur Scratch Texture Resource");
	}

#pragma endregion
}