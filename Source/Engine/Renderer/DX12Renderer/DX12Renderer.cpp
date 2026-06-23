#include "DX12Renderer.h"

#include "DDS Loader/DDSTextureLoader.h"
#include <DirectXColors.h>
#include "DX12 Upload BUffers/DX12DefaultUploadBuffer.h"
#include "DX12 Frame Resource/DX12FrameResource.h"
#include "DX12RendererHelper.h"
#include <dxgidebug.h>
#include <filesystem>
#include "../../../Helper/Helper.h"
#include "../../../Helper/Logger.h"
#include <WindowsX.h>

using namespace DirectX;

namespace Engine::EngineRenderer::DX12Renderer {

	DX12Renderer::DX12Renderer() {}

	DX12Renderer::~DX12Renderer() {

		if (mDX12Device != nullptr) {
			FlushCommandQueue();

			Shutdown();

			// report live objects to the debugger
	#if defined(_DEBUG) && !defined(__MINGW32__)
			{
				Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
				{
					dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
				}
			}
	#endif
		}
	}

	bool DX12Renderer::Initialize(
		HWND mainHWND, 
		int numberOfFrameResources,
		UINT screenWidth, 
		UINT screenHeight
	) {
		mhMainWnd = mainHWND;
		mNumberOfFrameResources = numberOfFrameResources;
		mWindowDimensions.Width = screenWidth;
		mWindowDimensions.Height = screenHeight;

		InitializeDevice();
		CreateCommandObjects();
		CreateSwapChain();
		CreateRtvDsvDescriptorHeaps();

		// Reset the command list to prep for initialization commands.
		ThrowIfFailed(mSetupCommandList->Reset(mInitAndResizeCommandAllocator.Get(), nullptr));

		return true;
	}

	bool DX12Renderer::InitializeDevice() {
		UINT dxgiFactoryFlags = 0;

	#if defined(DEBUG) || defined(_DEBUG) 
		// Enable the D3D12 debug layer.
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
		}

	#ifndef __MINGW32__
		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	#endif // __MINGW32__

	#endif

		// Initialize DXGI Factory
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

		// Try to create hardware device.
		HRESULT hardwareResult = D3D12CreateDevice(
			nullptr,             // default adapter
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&mDX12Device)
		);

		// Fallback to WARP device.
		if (FAILED(hardwareResult))
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
			ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter))
			);

			ThrowIfFailed(
				D3D12CreateDevice(
					pWarpAdapter.Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&mDX12Device)
				)
			);
		}

		// create fence
		ThrowIfFailed(
			mDX12Device->CreateFence(
				0,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&mFence)
			)
		);

		// Check 4X MSAA quality support for our back buffer format.
		// All Direct3D 11 capable devices support 4X MSAA for all render 
		// target formats, so we only need to check quality support.

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = mBackBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		ThrowIfFailed(
			mDX12Device->CheckFeatureSupport(
				D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
				&msQualityLevels,
				sizeof(msQualityLevels)
			)
		);

		m4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	#if defined(DEBUG) || defined(_DEBUG)
		LogAdapters();
	#endif

		return true;
	}

	void DX12Renderer::Shutdown() {
		mCurrentFrameResource = nullptr;
		for (auto& texture : mTextures) {
			if (texture.Resource != nullptr) {
				texture.Resource.Reset();
			}
		}
		for (auto& pair : mMeshResourceMap) {
			pair.second->VertexBufferGPU.Reset();
			pair.second->IndexBufferGPU.Reset();
		}
		mBlurPipelinePass.ShutDown();
		mDebugSystemPipelinePass.ShutDown();
		mRenderPipelinePass.ShutDown();
		for (auto& resource : mFrameResources) {
			resource.reset();
		}
		mDepthStencilBuffer.Reset();
		for (UINT i = 0; i < DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS; ++i) {
			mSwapChainBuffers[i].Reset();
		}
		mDSVDescriptorHeap.Reset();
		mRTVDescriptorHeap.Reset();
		mSwapChain.Reset();
		mSetupCommandList.Reset();
		mInitAndResizeCommandAllocator.Reset();
		mCommandQueue.Reset();
		mFence.Reset();
		mDX12Device.Reset();
		mdxgiFactory.Reset();
	}

	void DX12Renderer::PrepareForUpdate() {
		// Cycle through the circular frame resource array.
		mCurrentFrameResourceIndex = (mCurrentFrameResourceIndex + 1) % mNumberOfFrameResources;
		mCurrentFrameResource = mFrameResources[mCurrentFrameResourceIndex].get();

		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		if (mCurrentFrameResource->mFenceValue != 0 && mFence->GetCompletedValue() < mCurrentFrameResource->mFenceValue) {
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
			ThrowIfFailed(
				mFence->SetEventOnCompletion(
					mCurrentFrameResource->mFenceValue, 
					eventHandle
				)
			);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void DX12Renderer::UpdateOpaqueRenderItemsPerPassCb(
		const void* data, 
		size_t dataSize
	) const {
		mCurrentFrameResource->mOpaquePerPassCB.CopyData(0, data);
	}

	void DX12Renderer::UpdateOpaqueRenderItemCb(
		uint32_t renderItemIndex,
		const void* data, 
		uint32_t perRenderItemCbSize
	) {
		mCurrentFrameResource->mOpaqueRenderItemCB.CopyData(renderItemIndex, data);
	}

	void DX12Renderer::UpdatePerMaterialCb(
		uint32_t materialIndex,
		const void* data, 
		uint32_t perMaterialCbSize
	) {
		mCurrentFrameResource->mPerMaterialCB.CopyData(materialIndex, data);
	}

	void DX12Renderer::UpdateDebugSystemPerPassCb(const void* data) {
		mCurrentFrameResource->mDebugSystemPerPassCB.CopyData(0, data);
	}

	void DX12Renderer::UpdateDebugSystemStructuredBuffer(uint32_t count, const void* data) {
		mCurrentFrameResource->mDebugSystemPerCharacterCB.CopyData(count, data);
	}

	void DX12Renderer::BeginFrame(uint32_t numberOfMaterials) {
		auto commandAllocator = mCurrentFrameResource->mCommandAllocator;
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		ThrowIfFailed(commandAllocator->Reset());

		// We can reset our setup command list with a nullptr PSO 
		// because we're not using this command list for drawing
		// just transitioning our back buffer to render_target and
		// to the render target and depth stencil views
		ThrowIfFailed(
			mSetupCommandList->Reset(
				commandAllocator.Get(),
				nullptr
			)
		);
	
		// transition this frame's back buffer to render target
		ID3D12Resource* currentBackBuffer = mSwapChainBuffers[mCurrentBackBufferIndex].Get();

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			currentBackBuffer,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);

		// Indicate the transition via a command.
		mSetupCommandList->ResourceBarrier(
			1,
			&barrier
		);

		auto currentBackBufferView = CurrentBackBufferView();
		auto depthStencilView = DepthStencilView();

		// Clear the back buffer and depth/stencil buffer
		mSetupCommandList->ClearRenderTargetView(
			currentBackBufferView,
			Colors::LightSteelBlue,
			0, 
			nullptr
		);

		mSetupCommandList->ClearDepthStencilView(
			DepthStencilView(), 
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f, 
			0,
			0,
			nullptr
		);

		// Specify the buffers we are going to render to.
		mSetupCommandList->OMSetRenderTargets(
			1,
			&currentBackBufferView, 
			true,
			&depthStencilView
		);
	}

	void DX12Renderer::Execute(const IPipelinePassExecuteContext& context) {
		const DX12OpaquePipelinePassExecuteContext& ppContext = static_cast<const DX12OpaquePipelinePassExecuteContext&>(context);

		switch (context.GetPipelinePassType()) {
		case RendererPipelinePass::OPAQUE_RENDER_PASS:
			DrawOpaqueRenderItems(ppContext);
			break;
		case RendererPipelinePass::BLUR_UI_PASS:
			// todo:
			break;
		case RendererPipelinePass::DEBUG_SYSTEM_PASS:
			DrawDebugSystem(ppContext.NumberOfItems);
			break;
		default:
			Logger::PRINT(L"Warning NOT handling one of the cases of RendererPipelinePass in DX12Renderer");
		}
	}

	bool DX12Renderer::DrawOpaqueRenderItems(
		const DX12OpaquePipelinePassExecuteContext& context
	) {
		// todo: add this to some sort of a ring buffer so memory can be reused
		std::array<DX12OpaqueRenderPipelinePerItemExecuteArgs, DX12RendererConfig::MAX_ITEMS_PER_PASS> PerItemExecuteArgs = {};

		// fill in the execute args from execute context per render item
		for (uint32_t i = 0; i < context.NumberOfItems; ++i) {
			if (i >= DX12RendererConfig::MAX_ITEMS_PER_PASS) { break; }

			auto& renderItem = context.RenderItems[i];
			const DX12MeshResource* const resource = mMeshResourceMap[renderItem.MeshID].get();

			if (resource == nullptr) { continue; }

			PerItemExecuteArgs[i].ID = renderItem.ID;
			PerItemExecuteArgs[i].VertexBufferView = resource->VertexBufferView();
			PerItemExecuteArgs[i].IndexBufferView = resource->IndexBufferView();
			PerItemExecuteArgs[i].SubMeshCount = renderItem.SubMeshCount;

			// load up data per sub mesh per render item
			for (uint8_t j = 0; j < renderItem.SubMeshCount; ++j) {
				const DX12OpaqueRenderItemPerSubMeshExecuteContext& subMeshExecuteContext = renderItem.SubMeshExecuteContext[j];

				DX12OpaqueRenderPipelinePerItemPerSubMeshArgs& subMeshArgs = PerItemExecuteArgs[i].SubMeshExecuteArgs[j];

				subMeshArgs.ID = j;
				subMeshArgs.IndexCount = subMeshExecuteContext.IndexCount;
				subMeshArgs.StartIndexLocation = subMeshExecuteContext.StartIndexLocation;
				subMeshArgs.BaseVertexLocation = subMeshExecuteContext.BaseVertexLocation;
			}
		}

		ID3D12Resource* currentBackBuffer = mSwapChainBuffers[mCurrentBackBufferIndex].Get();
		auto backBufferView = CurrentBackBufferView();
		auto depthStencilView = DepthStencilView();
		
		DX12OpaqueRenderPipelineExecuteArgs rArgs{
			{
				mCurrentFrameResourceIndex,
				mCurrentBackBufferIndex,
				mCbvSrvUavDescriptorSize,
				currentBackBuffer,
				backBufferView,
				depthStencilView,
				mScreenViewport,
				mScissorRect
			},
			PerItemExecuteArgs.data(),
			context.NumberOfItems,
			context.NumOfSubMeshesPerItem,
			mCurrentFrameResource->mOpaquePerPassCB.Resource()->GetGPUVirtualAddress(),
			mCurrentFrameResource->mOpaqueRenderItemCB.ElementByteSize(),
			mCurrentFrameResource->mOpaqueRenderItemCB.Resource()->GetGPUVirtualAddress(),
			mCurrentFrameResource->mOpaqueRenderItemPerSubMeshCB.ElementByteSize(),
			mCurrentFrameResource->mOpaqueRenderItemPerSubMeshCB.Resource()->GetGPUVirtualAddress(),
			context.NumberOfMaterials
		};

		mRenderPipelinePass.ExecutePass(rArgs);

		mPiplinePassAggregator.InsertPass(&mRenderPipelinePass);

		return true;
	}

	bool DX12Renderer::DrawDebugSystem(uint32_t numberOfCharacters) {
		// todo: remove this from here in the future
		DrawBlurPass();

		ID3D12Resource* currentBackBuffer = mSwapChainBuffers[mCurrentBackBufferIndex].Get();
		auto backBufferView = CurrentBackBufferView();
		auto depthStencilView = DepthStencilView();

		DX12DebugSystemPipelinePassExecuteArgs args {
			{
				mCurrentFrameResourceIndex,
				mCurrentBackBufferIndex,
				mCbvSrvUavDescriptorSize,
				currentBackBuffer,
				backBufferView,
				depthStencilView,
				mScreenViewport,
				mScissorRect
			},
			mCurrentFrameResource->mDebugSystemPerPassCB.Resource(),
			numberOfCharacters
		};

		mDebugSystemPipelinePass.ExecutePass(args);

		mPiplinePassAggregator.InsertPass(&mDebugSystemPipelinePass);

		return true;
	}

	void DX12Renderer::EndFrame() {
		DX12PipelinePassAggregatorResult result;
		mPiplinePassAggregator.Aggregate(mSetupCommandList.Get(), result);

		// Record the transition to present directly onto the last command list
		// be it the main one (for now) or one of the different ones
		ID3D12Resource* currentBackBuffer = mSwapChainBuffers[mCurrentBackBufferIndex].Get();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			currentBackBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		ID3D12GraphicsCommandList* lastCommandList = (ID3D12GraphicsCommandList*)result.ActiveLists.back();
		lastCommandList->ResourceBarrier(1, &barrier);

		HRESULT hr = lastCommandList->Close();
		if (FAILED(hr)) {
			Logger::ERR(L"Closing the command list has failed. This should NOT happeen");
			ThrowException(hr);
		}

		// reset for the next frame
		mPiplinePassAggregator.Reset();

		// Add the command lists to the queue for execution
		mCommandQueue->ExecuteCommandLists(
			(UINT)result.ActiveLists.size(),
			result.ActiveLists.data()
		);
		
		// mark the back buffer ready for presentation
		ThrowIfFailed(mSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));

		// swap the back and front buffers
		mCurrentBackBufferIndex = (mCurrentBackBufferIndex + 1) % DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS;

		// Advance the fence value to mark commands up to this fence point.
		mCurrentFrameResource->mFenceValue = ++mCurrentFence;

		// Add an instruction to the command queue to set a new fence point. 
		// Because we are on the GPU timeline, the new fence point won't be 
		// set until the GPU finishes processing all the commands prior to this Signal().
		mCommandQueue->Signal(mFence.Get(), mCurrentFence);
	}

	void DX12Renderer::OnResize(UINT width, UINT height) {
		// make sure we have a valid device
		if (mDX12Device == nullptr) { return; }

		mWindowDimensions.Width = width;
		mWindowDimensions.Height = height;

		// make sure we have a valid swap chain and command allocator
		assert(mSwapChain);
		assert(mInitAndResizeCommandAllocator);

		// flush all preivous commands
		FlushCommandQueue();

		// reset the command allocator
		ThrowIfFailed(mSetupCommandList->Reset(mInitAndResizeCommandAllocator.Get(), nullptr));

		// reset the back buffers
		for (int i = 0; i < DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS; ++i) {
			mSwapChainBuffers[i].Reset();
		}
		// reset the depth stencil buffer
		mDepthStencilBuffer.Reset();

		// resize the actual back buffers
		ThrowIfFailed(
			mSwapChain->ResizeBuffers(
				DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS,
				width,
				height,
				mBackBufferFormat,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
			)
		);

		mCurrentBackBufferIndex = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE mRTVHeapHandle(
			mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		);

		// recreate the back buffer views
		for (int i = 0; i < DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS; ++i) {
			ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffers[i])));
			mDX12Device->CreateRenderTargetView(
				mSwapChainBuffers[i].Get(), 
				nullptr,
				mRTVHeapHandle
			);

			mSwapChainBuffers[i].Get()->SetName(
				(L"Swap Chain Back Buffer View: " + std::to_wstring(i)).c_str()
			);
			mRTVHeapHandle.Offset(1, mRtvDescriptorSize);
		}

		// clear the blur scratch texture and recreate the teture and the descriptors
		// this is because our back buffer and scratch texture both have been
		// recreated with the new screen dimensions
		std::array<ID3D12Resource*, DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS> SwapchainBuffers;

		for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS; ++i) {
			SwapchainBuffers[i] = mSwapChainBuffers[i].Get();
		}

		DX12BlurPipelinePassInitArgs bArgs{
			{
				mDX12Device.Get(),
				mInitAndResizeCommandAllocator.Get(),
				mBackBufferFormat,
				mDepthStencilFormat,
				mCbvSrvUavDescriptorSize
			},
			DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS,
			SwapchainBuffers,
			mWindowDimensions.Width,
			mWindowDimensions.Height
		};
		mBlurPipelinePass.OnResize(width, height, bArgs);

		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;

		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = mDepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		ThrowIfFailed(
			mDX12Device->CreateCommittedResource(
				&properties,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&optClear,
				IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
			)
		);
		mDepthStencilBuffer.Get()->SetName(L"Depth Stencil Buffer");

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = mDepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		mDX12Device->CreateDepthStencilView(
			mDepthStencilBuffer.Get(), 
			&dsvDesc, 
			mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		);

		// Transition the resource from its initial state to be used as a depth buffer.
		auto depthBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, 
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		);
		mSetupCommandList->ResourceBarrier(1, &depthBufferBarrier);

		// close the command list and execute all commands we're recorded here
		ThrowIfFailed(mSetupCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { mSetupCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// flush the command queue before we begin drawing
		FlushCommandQueue();

		// Update the viewport transform to cover the client area.
		mScreenViewport.TopLeftX = 0;
		mScreenViewport.TopLeftY = 0;
		mScreenViewport.Width = static_cast<float>(width);
		mScreenViewport.Height = static_cast<float>(height);
		mScreenViewport.MinDepth = 0.0f;
		mScreenViewport.MaxDepth = 1.0f;

		mScissorRect = { 0, 0, (long)width, (long)height };
	}

	void DX12Renderer::FlushCommandQueue() {
		// Advance the fence value to mark commands up to this fence point.
		mCurrentFence++;

		// Add an instruction to the command queue to set a new fence point.  Because we 
		// are on the GPU timeline, the new fence point won't be set until the GPU finishes
		// processing all the commands prior to this Signal().
		ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

		// Wait until the GPU has completed commands up to this fence point.
		if (mFence->GetCompletedValue() < mCurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

			// Fire event when GPU hits current fence.  
			ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

			// Wait until the GPU hits current fence event is fired.
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	void DX12Renderer::LogAdapters() {
		UINT i = 0;
		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;
		while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L"\n";

			Logger::PRINT(text);

			adapterList.push_back(adapter);

			++i;
		}

		for (size_t i = 0; i < adapterList.size(); ++i)
		{
			LogAdapterOutputs(adapterList[i]);
			adapterList[i]->Release();
			adapterList[i] = 0;
		}
	}

	void DX12Renderer::LogAdapterOutputs(IDXGIAdapter* adapter)
	{
		UINT i = 0;
		IDXGIOutput* output = nullptr;
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);

			std::wstring text = L"***Output: ";
			text += desc.DeviceName;
			text += L"\n";
			Logger::PRINT(text);

			LogOutputDisplayModes(output, mBackBufferFormat);

			output->Release();
			output = 0;

			++i;
		}
	}

	void DX12Renderer::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{
		UINT count = 0;
		UINT flags = 0;

		// Call with nullptr to get list count.
		output->GetDisplayModeList(format, flags, &count, nullptr);

		std::vector<DXGI_MODE_DESC> modeList(count);
		output->GetDisplayModeList(format, flags, &count, &modeList[0]);

		for (auto& x : modeList)
		{
			UINT n = x.RefreshRate.Numerator;
			UINT d = x.RefreshRate.Denominator;
			std::wstring text =
				L"Width = " + std::to_wstring(x.Width) + L" " +
				L"Height = " + std::to_wstring(x.Height) + L" " +
				L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
				L"\n";

			Logger::PRINT(text);
		}
	}

	void DX12Renderer::CreateCommandObjects() {

		// describe the command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		// create the command queue
		ThrowIfFailed(
			mDX12Device->CreateCommandQueue(
				&queueDesc,
				IID_PPV_ARGS(&mCommandQueue)
			)
		);
	
		// set the name of the command queue
		mCommandQueue->SetName(L"Command Queue");

		// create init and resize command allocator
		ThrowIfFailed(
			mDX12Device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(mInitAndResizeCommandAllocator.GetAddressOf())
			)
		);

		// set the name of the init and resize command allocator
		mInitAndResizeCommandAllocator->SetName(L"Init and Resize Command Allocator");

		// create command list
		ThrowIfFailed(
			mDX12Device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				mInitAndResizeCommandAllocator.Get(), // Associated command allocator
				nullptr,                              // Initial PipelineStateObject
				IID_PPV_ARGS(mSetupCommandList.GetAddressOf())
			)
		);

		// set the name of the command list
		mSetupCommandList->SetName(L"Command List");

		// Start off in a closed state.  This is because the first time we refer 
		// to the command list we will Reset it, and it needs to be closed before
		// calling Reset.
		mSetupCommandList->Close();
	}

	void DX12Renderer::CreateSwapChain() {
		// Release the previous swapchain as we will be recreating.
		mSwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = mWindowDimensions.Width;
		sd.Height = mWindowDimensions.Height;
		sd.Format = mBackBufferFormat;
		sd.Stereo = FALSE;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		sd.BufferCount = DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
		ThrowIfFailed(mdxgiFactory.As(&factory2));

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(
			factory2->CreateSwapChainForHwnd(
				mCommandQueue.Get(),
				mhMainWnd,
				&sd,
				nullptr,
				nullptr,
				&swapChain1
			)
		);

		ThrowIfFailed(swapChain1.As(&mSwapChain));

		mSwapChain->SetPrivateData(
			WKPDID_D3DDebugObjectName,
			sizeof("Swap Chain") - 1,
			"Swap Chain"
		);

		// Prevent DXGI from monitoring the message queue and hijacking Alt+Enter 
		// This ensures our custom F and ESC windowing states function correctly.
		ThrowIfFailed(mdxgiFactory->MakeWindowAssociation(
			mhMainWnd,
			DXGI_MWA_NO_ALT_ENTER
		));
	}

	void DX12Renderer::CreateRtvDsvDescriptorHeaps() {
		// get rtv, dsv, and srv descriptor size
		mRtvDescriptorSize = mDX12Device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
		mDsvDescriptorSize = mDX12Device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV
		);
		mCbvSrvUavDescriptorSize = mDX12Device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

		// describe the rtv heap
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;

		// create rtv heap
		ThrowIfFailed(
			mDX12Device->CreateDescriptorHeap(
				&rtvHeapDesc, 
				IID_PPV_ARGS(mRTVDescriptorHeap.GetAddressOf())
			)
		);

		// decribe the dsv heap
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;

		// create the dsv heap
		ThrowIfFailed(
			mDX12Device->CreateDescriptorHeap(
				&dsvHeapDesc, 
				IID_PPV_ARGS(mDSVDescriptorHeap.GetAddressOf())
			)
		);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::CurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrentBackBufferIndex,
			mRtvDescriptorSize
		);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::DepthStencilView() const
	{
		return mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	}

	bool DX12Renderer::SetupOpaqueRenderPipeline(
		uint32_t numberOfEntities, 
		uint8_t maxSubMeshesPerEntity,
		uint32_t numberOfMaterials,
		uint32_t numberOfTextures,
		uint32_t sizeOfPerMaterialCb,
		uint32_t debugSystemPerPassCBCount,
		uint32_t debugSystemMaxCharacters
	) {
		CreateFrameResources(
			numberOfEntities,
			maxSubMeshesPerEntity,
			numberOfMaterials,
			debugSystemPerPassCBCount,
			debugSystemMaxCharacters
		);

		if (mRenderPipelinePass.GetIsInitialized()) { return true; }

		std::array<D3D12_GPU_VIRTUAL_ADDRESS, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES> PerMaterialCBAddress;

		for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++i) {
			PerMaterialCBAddress[i] = mFrameResources[i]->mPerMaterialCB.Resource()->GetGPUVirtualAddress();
		}
		uint32_t alignedSizeOfPerMaterialCb = DX12RendererHelper::CalculateAlignedConstantBufferByteSize(sizeOfPerMaterialCb);

		DX12OpaqueRenderPipelineInitArgs rArgs{
			{
				mDX12Device.Get(),
				mInitAndResizeCommandAllocator.Get(),
				mBackBufferFormat,
				mDepthStencilFormat,
				mCbvSrvUavDescriptorSize
			},
			numberOfMaterials,
			numberOfTextures,
			alignedSizeOfPerMaterialCb,
			PerMaterialCBAddress,
			mTextures.data()
		};

		if (!mRenderPipelinePass.Initialize(rArgs)) { return false; }

		return true;
	}

	void DX12Renderer::CreateFrameResources(
		uint32_t numberOfEntities,
		uint8_t maxSubMeshesPerEntity,
		uint32_t numberOfMaterials,
		uint32_t debugSystemPerPassCBCount,
		uint32_t debugSystemMaxCharacters
	) {
		for (UINT i = 0; i < mNumberOfFrameResources; ++i) {
			mFrameResources.push_back(
				std::make_unique<DX12FrameResource>(
					mDX12Device.Get(),
					1,
					numberOfEntities,
					maxSubMeshesPerEntity,
					numberOfMaterials,
					debugSystemPerPassCBCount,
					debugSystemMaxCharacters
				)
			);
		}
	}

	bool DX12Renderer::SetupDebugPipeline(
		uint32_t debugSystemMaxCharacters,
		uint32_t fontAtlasIndex
	) {
		if (mDebugSystemPipelinePass.GetIsInitialized()) { return true; }

		if (fontAtlasIndex >= mTextures.size() || !mTextures[fontAtlasIndex].IsLoaded) {
			return false; 
		}

		std::array<ID3D12Resource*, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES> CBResources;

		for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++i) {
			CBResources[i] = mFrameResources[i]->mDebugSystemPerCharacterCB.Resource();
		}

		DX12DebugSystemPipelinePassInitArgs dArgs{
			{
				mDX12Device.Get(),
				mInitAndResizeCommandAllocator.Get(),
				mBackBufferFormat,
				mDepthStencilFormat,
				mCbvSrvUavDescriptorSize
			},
			debugSystemMaxCharacters,
			fontAtlasIndex,
			mTextures[fontAtlasIndex],
			CBResources
		};

		if (!mDebugSystemPipelinePass.Initialize(dArgs)) { return false; }

		return true;
	}

	bool DX12Renderer::SetupBlurPipeline() {
		if (mBlurPipelinePass.GetIsInitialized()) { return true; }

		std::array<ID3D12Resource*, DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS> swapChainBuffers;

		for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS; ++i) {
			swapChainBuffers[i] = mSwapChainBuffers[i].Get();
		}

		DX12BlurPipelinePassInitArgs bArgs{
			{
				mDX12Device.Get(),
				mInitAndResizeCommandAllocator.Get(),
				mBackBufferFormat,
				mDepthStencilFormat,
				mCbvSrvUavDescriptorSize
			},
			DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS,
			swapChainBuffers,
			mWindowDimensions.Width,
			mWindowDimensions.Height
		};

		if (!mBlurPipelinePass.Initialize(bArgs)) { return false; }

		return true;
	}

	bool DX12Renderer::DrawBlurPass() {

		ID3D12Resource* currentBackBuffer = mSwapChainBuffers[mCurrentBackBufferIndex].Get();
		auto backBufferView = CurrentBackBufferView();
		auto depthStencilView = DepthStencilView();

		DX12BlurPipelinePassExecuteArgs args{
			{
				mCurrentFrameResourceIndex,
				mCurrentBackBufferIndex,
				mCbvSrvUavDescriptorSize,
				currentBackBuffer,
				backBufferView,
				depthStencilView,
				mScreenViewport,
				mScissorRect
			},
			mWindowDimensions.Width,
			mWindowDimensions.Height
		};

		mBlurPipelinePass.ExecutePass(args);

		mPiplinePassAggregator.InsertPass(&mBlurPipelinePass);

		return true;
	}

	void DX12Renderer::LoadGeometry(
		uint32_t meshID, 
		uint16_t sizeOfVertex,
		uint32_t vertexBufferByteSize, 
		const void* vertices, 
		uint32_t indexBufferByteSize,
		const void* indices
	) {
		// create mesh resource
		std::unique_ptr<DX12MeshResource> meshResource = std::make_unique<DX12MeshResource>();
		meshResource->id = meshID;

		// create blob of vbByteSize and store address in vertex buffer cpu address
		ThrowIfFailed(
			D3DCreateBlob(
				vertexBufferByteSize,
				&meshResource->VertexBufferCPU
			)
		);

		// copy vertices into vertex buffer cpu address
		CopyMemory(
			meshResource->VertexBufferCPU->GetBufferPointer(),
			vertices,
			vertexBufferByteSize
		);

		// create blob and copy indices into blob at index buffer cpu address
		ThrowIfFailed(
			D3DCreateBlob(
				indexBufferByteSize,
				&meshResource->IndexBufferCPU
			)
		);

		CopyMemory(
			meshResource->IndexBufferCPU->GetBufferPointer(),
			indices,
			indexBufferByteSize
		);

		// create default buffer on the gpu and upload vertices and indices
		// from cpu to gpu usign the default buffer
		meshResource->VertexBufferGPU = DX12RendererHelper::CreateDefaultBuffer(
			mDX12Device.Get(),
			mSetupCommandList.Get(),
			vertices,
			vertexBufferByteSize,
			meshResource->VertexBufferUploader
		);

		meshResource->IndexBufferGPU = DX12RendererHelper::CreateDefaultBuffer(
			mDX12Device.Get(),
			mSetupCommandList.Get(), 
			indices,
			indexBufferByteSize,
			meshResource->IndexBufferUploader
		);

		// store metadata in mesh resource that we'll use during rendering
		meshResource->VertexByteStride = sizeOfVertex;
		meshResource->VertexBufferByteSize = vertexBufferByteSize;
		meshResource->IndexFormat = DXGI_FORMAT_R16_UINT;
		meshResource->IndexBufferByteSize = indexBufferByteSize;

		// store it in our map
		mMeshResourceMap[meshID] = std::move(meshResource);
	}

	bool DX12Renderer::LoadTexture(std::wstring& filename, uint32_t id) {
		if (id >= mTextures.size()) { return false; }

		DX12Texture& texture = mTextures[id];

		if (texture.IsLoaded) { return false; }

		texture.Id = id;

		ThrowIfFailed(
			DirectX::CreateDDSTextureFromFile12(
				mDX12Device.Get(),
				mSetupCommandList.Get(),
				filename.c_str(),
				texture.Resource,
				texture.UploadHeap
			)
		);

		texture.IsLoaded = true;

		return true;
	}

	void DX12Renderer::FinishInitialize() {
		// Execute the initialization commands.
		ThrowIfFailed(mSetupCommandList->Close());

		ID3D12CommandList* cmdsLists[] = { mSetupCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until initialization is complete.
		FlushCommandQueue();

		// dispose uploaders
		DisposeUploaders();
	}

	void DX12Renderer::DisposeUploaders() {

		// dispose mesh uploaders
		for (auto it = mMeshResourceMap.begin(); it != mMeshResourceMap.end(); ++it) {
			uint32_t id = it->first;
			auto& meshResource = it->second;

			meshResource->DisposeUploaders();
		}

		// dispose texture uploaders
		for (DX12Texture& texture : mTextures) {
			if (texture.IsLoaded && texture.UploadHeap != nullptr) {
				texture.DisposeUploader();
			}
		}
	}
}