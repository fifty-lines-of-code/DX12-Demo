#include "DX12Renderer.h"

#include "DDS Loader/DDSTextureLoader.h"
#include <DirectXColors.h>
#include "DX12DefaultUploadBuffer.h"
#include "DX12FrameResource.h"
#include "DX12RendererHelper.h"
#include <dxgidebug.h>
#include <filesystem>
#include "../../../Helper/Helper.h"
#include "../../../Helper/Logger.h"
#include <WindowsX.h>

using namespace DirectX;

DX12Renderer::DX12Renderer() {}

DX12Renderer::~DX12Renderer() {

	if (mDX12Device != nullptr) {
		FlushCommandQueue();

		Shutdown();

		// report live objects to the debugger
#if defined(_DEBUG) && !defined(__MINGW32__)
		{
			ComPtr<IDXGIDebug1> dxgiDebug;
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
	ThrowIfFailed(mCommandList->Reset(mInitAndResizeCommandAllocator.Get(), nullptr));

	return true;
}

bool DX12Renderer::InitializeDevice() {
	UINT dxgiFactoryFlags = 0;

#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}

#ifndef __MINGW32__
	ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
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
		ComPtr<IDXGIAdapter> pWarpAdapter;
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
	for (auto& resource : mFrameResources) {
		resource.reset();
	}
	mCurrentFrameResource = nullptr;
	for (auto it = mMeshResourceMap.begin(); it != mMeshResourceMap.end(); ++it) {
		uint32_t id = it->first;
		auto& meshResource = it->second;

		meshResource->VertexBufferGPU.Reset();
		meshResource->IndexBufferGPU.Reset();
	}
	mPipelineStateObject.Reset();
	mpsByteCode.Reset();
	mvsByteCode.Reset();
	mRootSignature.Reset();
	mDepthStencilBuffer.Reset();
	for (UINT i = 0; i < SwapChainBufferCount; ++i) {
		mSwapChainBuffer[i].Reset();
	}
	for (auto texture : mTextures) {
		if (texture.Resource != nullptr) {
			texture.Resource.Reset();
		}
	}
	mCBVSRVDescriptorHeap.Reset();
	mDSVDescriptorHeap.Reset();
	mRTVDescriptorHeap.Reset();
	mSwapChain.Reset();
	mCommandList.Reset();
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


void DX12Renderer::UpdatePerPassCb(void* data, size_t dataSize) const {
	mCurrentFrameResource->mPerPassCB.CopyData(0, data);
}

void DX12Renderer::UpdatePerRenderItemCb(uint32_t renderItemIndex, void* data, uint32_t perRenderItemCbSize) {
	mCurrentFrameResource->mPerRenderItemCB.CopyData(renderItemIndex, data);
}

void DX12Renderer::UpdatePerMaterialCb(uint32_t materialIndex, void* data, uint32_t perMaterialCbSize) {
	mCurrentFrameResource->mPerMaterialCB.CopyData(materialIndex, data);
}

void DX12Renderer::UpdateDebugSystemPerPassCb(void* data) {
	mCurrentFrameResource->mDebugSystemPerPassCB.CopyData(0, data);
}

void DX12Renderer::UpdateDebugSystemStructuredBuffer(uint32_t count, const void* data) {
	mCurrentFrameResource->mDebugSystemPerCharacterCB.CopyData(count, data);
}

void DX12Renderer::BeginFrame(uint32_t numberOfMaterials) {
	auto commandAllocator = mCurrentFrameResource->mCommandListAllocator;
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(commandAllocator->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(
		commandAllocator.Get(),
		mPipelineStateObject.Get())
	);

	// set viewports and scissor rect
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);
	
	// transition this frame's back buffer to render target
	ID3D12Resource* currentBackBuffer = mSwapChainBuffer[mCurrentBackBuffer].Get();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(
		1,
		&barrier
	);

	// Clear the back buffer and depth/stencil buffer
	mCommandList->ClearRenderTargetView(
		CurrentBackBufferView(),
		Colors::LightSteelBlue,
		0, 
		nullptr
	);

	mCommandList->ClearDepthStencilView(
		DepthStencilView(), 
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 
		0,
		0,
		nullptr
	);

	// Specify the buffers we are going to render to.
	auto currentBackBufferView = CurrentBackBufferView();
	auto depthStencilView = DepthStencilView();

	mCommandList->OMSetRenderTargets(
		1,
		&currentBackBufferView, 
		true,
		&depthStencilView
	);

	// set the per pass cb, materials cb
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCBVSRVDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the root signature
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	// per pass cb
	auto passCB = mCurrentFrameResource->mPerPassCB.Resource();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress();
	mCommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);

	// materials cb
	auto materialsCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBVSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	int globalMaterialHeapOffset = (numberOfMaterials * mCurrentFrameResourceIndex);
	materialsCbvHandle.Offset(globalMaterialHeapOffset, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(2, materialsCbvHandle);

	// textures
	auto texturesCbHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBVSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	texturesCbHandle.Offset(mTexturesCbHeapOffset, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(3, texturesCbHandle);
}

bool DX12Renderer::Draw(uint32_t meshID, uint32_t indexCount, uint32_t entityIndex, uint32_t entityCount) {
	const DX12MeshResource* const resource = mMeshResourceMap[meshID].get();

	if (resource == nullptr) { return false; }

	auto vertexBufferView = resource->VertexBufferView();
	mCommandList->IASetVertexBuffers(
		0, 
		1, 
		&vertexBufferView
	);

	auto indexBufferView = resource->IndexBufferView();
	mCommandList->IASetIndexBuffer(
		&indexBufferView
	);

	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Offset to the CBV in the CBV heap for this object and for this frame resource.
	auto& renderItemCB = mCurrentFrameResource->mPerRenderItemCB;
	UINT elementByteSize = renderItemCB.ElementByteSize();
	auto renderItemCBResource = renderItemCB.Resource();
	D3D12_GPU_VIRTUAL_ADDRESS renderItemCBAddress = renderItemCBResource->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS currentEntityAddress = renderItemCBAddress + (entityIndex * elementByteSize);

	mCommandList->SetGraphicsRootConstantBufferView(0, currentEntityAddress);

	mCommandList->DrawIndexedInstanced(
		indexCount,
		1, 0, 0, 0
	);

	return true;
}

bool DX12Renderer::DrawDebugSystem(uint32_t numberOfCharacters) {
	// Grab the active command list for the current frame
	auto cmdList = mCommandList.Get();

	cmdList->SetPipelineState(mDebugPipelineStateObject.Get());
	cmdList->SetGraphicsRootSignature(mDebugRootSignature.Get());

	// 1. Bind ONLY the debug heap—holding both the structured buffer and the font copy!
	ID3D12DescriptorHeap* activeHeaps[] = { mDebugCBVSRVDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(1, activeHeaps);

	// 2. Parameter 0: Direct Virtual Address for Constant Buffer
	auto perPassResource = mFrameResources[mCurrentFrameResourceIndex]->mDebugSystemPerPassCB.Resource();
	cmdList->SetGraphicsRootConstantBufferView(0, perPassResource->GetGPUVirtualAddress());

	// 3. Parameter 1: Structured Buffer table
	CD3DX12_GPU_DESCRIPTOR_HANDLE sbHandle(mDebugCBVSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	sbHandle.Offset(mCurrentFrameResourceIndex, mCbvSrvUavDescriptorSize);
	cmdList->SetGraphicsRootDescriptorTable(1, sbHandle);

	// 4. Parameter 2: Font Texture table
	CD3DX12_GPU_DESCRIPTOR_HANDLE fontHandle(mDebugCBVSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	fontHandle.Offset(mNumberOfFrameResources, mCbvSrvUavDescriptorSize);
	cmdList->SetGraphicsRootDescriptorTable(2, fontHandle);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 5. Fire off the procedural magic
	cmdList->DrawInstanced(4, numberOfCharacters, 0, 0);

	return true;
}

void DX12Renderer::EndFrame() {
	ID3D12Resource* currentBackBuffer = mSwapChainBuffer[mCurrentBackBuffer].Get();

	// transition the current back buffer to present
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		currentBackBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &barrier);

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrentFrameResource->mFenceValue = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void DX12Renderer::OnResize(UINT width, UINT height) {
	if (mDX12Device == nullptr) { return; }

	mWindowDimensions.Width = width;
	mWindowDimensions.Height = height;

	// ensure if we have devicewe also have swap chain, allocator
	assert(mSwapChain);
	assert(mInitAndResizeCommandAllocator);

	// flush before changing any resources
	FlushCommandQueue();

	// reset command list allocator
	ThrowIfFailed(
		mCommandList->Reset(mInitAndResizeCommandAllocator.Get(), nullptr)
	);

	// release the previous resources
	for (int i = 0; i < SwapChainBufferCount; ++i) {
		mSwapChainBuffer[i].Reset();
	}
	mDepthStencilBuffer.Reset();

	// resize the swap chain
	ThrowIfFailed(
		mSwapChain->ResizeBuffers(
			SwapChainBufferCount,
			width,
			height,
			mBackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
		)
	);

	mCurrentBackBuffer = 0;

	// create the rtv for each buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE mRTVHeapHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	);
	for (int i = 0; i < SwapChainBufferCount; ++i) {
		ThrowIfFailed(
			mSwapChain->GetBuffer(
				i,
				IID_PPV_ARGS(&mSwapChainBuffer[i])
			)
		);
		mDX12Device->CreateRenderTargetView(
			mSwapChainBuffer[i].Get(),
			nullptr,
			mRTVHeapHandle
		);
		mRTVHeapHandle.Offset(1, mRtvDescriptorSize);
	}

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

	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
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
	mCommandList->ResourceBarrier(
		1,
		&depthBufferBarrier
	);

	// Execute the resize commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
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

	// create init and resize command allocator
	ThrowIfFailed(
		mDX12Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(mInitAndResizeCommandAllocator.GetAddressOf())
		)
	);

	// create command list
	ThrowIfFailed(
		mDX12Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mInitAndResizeCommandAllocator.Get(), // Associated command allocator
			nullptr,                   // Initial PipelineStateObject
			IID_PPV_ARGS(mCommandList.GetAddressOf())
		)
	);

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	mCommandList->Close();
}

void DX12Renderer::CreateSwapChain() {
	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mWindowDimensions.Width;
	sd.BufferDesc.Height = mWindowDimensions.Height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(
		mdxgiFactory->CreateSwapChain(
			mCommandQueue.Get(),
			&sd,
			mSwapChain.GetAddressOf()
		)
	);

	// Prevent DXGI from monitoring the message queue and hijacking Alt+Enter 
	// This ensures our custom F and ESC windowing states function correctly.
	ThrowIfFailed(
		mdxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER)
	);
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
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
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
		mCurrentBackBuffer,
		mRtvDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Renderer::DepthStencilView() const
{
	return mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

bool DX12Renderer::SetupPipeline(
	uint32_t numberOfEntities, 
	uint32_t numberOfMaterials,
	uint32_t numberOfTextures,
	uint32_t sizeOfPerMaterialCb,
	uint32_t debugSystemPerPassCBCount,
	uint32_t debugSystemMaxCharacters
) {
	CreateFrameResources(numberOfEntities, numberOfMaterials, debugSystemPerPassCBCount, debugSystemMaxCharacters);
	if (!CreateConstantBufferDescriptor(numberOfMaterials, numberOfTextures, sizeOfPerMaterialCb)) { return false; }
	if (!CreateRootSignature(numberOfMaterials, numberOfTextures)) { return false; }
	if (!CreateShadersAndInputLayout()) { return false; }
	if (!CreatePipelineStateObject()) { return false; }

	return true;
}
void DX12Renderer::CreateFrameResources(uint32_t numberOfEntities, uint32_t numberOfMaterials, uint32_t debugSystemPerPassCBCount, uint32_t debugSystemMaxCharacters) {
	for (UINT i = 0; i < mNumberOfFrameResources; ++i) {
		mFrameResources.push_back(
			std::make_unique<DX12FrameResource>(
				mDX12Device.Get(),
				1,
				numberOfEntities,
				numberOfMaterials,
				debugSystemPerPassCBCount,
				debugSystemMaxCharacters
			)
		);
	}
}

bool DX12Renderer::CreateConstantBufferDescriptor(
	uint32_t numberOfMaterials,
	uint32_t numberOfTextures,
	uint32_t sizeOfPerMaterialCb
) {
	uint32_t alignedSizeOfPerMaterialCb = DX12RendererHelper::CalculateAlignedConstantBufferByteSize(sizeOfPerMaterialCb);

	// Textures sit at the very end after the 3 frames of per material cb
	mTexturesCbHeapOffset = numberOfMaterials * mNumberOfFrameResources;

	// Compute the global total of descriptors across all frames
	// (1 pass + N entities + M materials) * Total Frames + No Textures
	UINT numberOfDescriptors = numberOfMaterials * mNumberOfFrameResources + numberOfTextures;

	// Describe the CBV descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numberOfDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	// Create the CBV descriptor heap
	ThrowIfFailed(
		mDX12Device->CreateDescriptorHeap(
			&cbvHeapDesc,
			IID_PPV_ARGS(&mCBVSRVDescriptorHeap)
		)
	);

	// Pass the offsets and sizes forward to generate the views
	return CreateConstantBufferViews(
		numberOfMaterials,
		numberOfTextures,
		alignedSizeOfPerMaterialCb
	);
}

bool DX12Renderer::CreateConstantBufferViews(
	uint32_t numberOfMaterials,
	uint32_t numberOfTextures,
	uint32_t alignedSizeOfPerMaterialCb
) {
	// per material cbs are laid out first per frame
	// so 
	// PerMatCB0(F0), PerMatCB1(F0), PerMatCB0(F1)..., PerMatCBN-1(FN-1)
	// then the textures are laid out
	// T0, T1..., TN-1

	// our per material cb
	// ((alignedPerMaterialCB) * numberOfMaterials * numberOfFrames
	for (UINT frameIndex = 0; frameIndex < mNumberOfFrameResources; ++frameIndex)
	{
		auto materialCB = mFrameResources[frameIndex]->mPerMaterialCB.Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = materialCB->GetGPUVirtualAddress();

		for (uint32_t i = 0; i < numberOfMaterials; ++i) {
			// Offset to this material cbv in the descriptor heap.
			int heapIndex = (numberOfMaterials * frameIndex) + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCBVSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress + (i * alignedSizeOfPerMaterialCb);
			cbvDesc.SizeInBytes = alignedSizeOfPerMaterialCb;

			mDX12Device->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	// then our textures

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mCBVSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	// offset to the start of textures
	hDescriptor.Offset(mTexturesCbHeapOffset, mCbvSrvUavDescriptorSize);

	for (uint32_t i = 0; i < numberOfTextures; ++i) {
		// walk to this descriptor in the heap
		CD3DX12_CPU_DESCRIPTOR_HANDLE currentHandle(hDescriptor, (INT)i, mCbvSrvUavDescriptorSize);

		DX12Texture& tex = mTextures[i];
		ID3D12Resource* resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		// if unloaded 
		if (!tex.IsLoaded || tex.Resource == nullptr) {
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Generic pixel format
			srvDesc.Texture2D.MipLevels = 1;
			resource = nullptr;
		}
		else {
			srvDesc.Format = tex.Resource->GetDesc().Format; // Grab format from the DDS file
			srvDesc.Texture2D.MipLevels = tex.Resource->GetDesc().MipLevels;
			resource = tex.Resource.Get();
		}

		// IF THE SLOT IS USED: Create the real Shader Resource View
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		// Bake the view configuration straight into the descriptor heap slot
		mDX12Device->CreateShaderResourceView(resource, &srvDesc, currentHandle);
	}

	return true;
}

bool DX12Renderer::CreateRootSignature(uint32_t numberOfMaterials, uint32_t numberOfTextures) {
	// per material cb
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, numberOfMaterials, 2);

	// textures buffer
	CD3DX12_DESCRIPTOR_RANGE cbvTable2;
	cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numberOfTextures, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Create a two descriptor tables of CBVs.
	// per entity cb
	slotRootParameter[0].InitAsConstantBufferView(0);
	// per pass cb
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable1);
	slotRootParameter[3].InitAsDescriptorTable(1, &cbvTable2, D3D12_SHADER_VISIBILITY_PIXEL);

	std::array<CD3DX12_STATIC_SAMPLER_DESC, DX12RendererHelper::DX12_MAX_SAMPLERS> samplers;
	DX12RendererHelper::GetStaticSamplers(samplers);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init(
		_countof(slotRootParameter),
		slotRootParameter,
		(UINT)samplers.size(),
		samplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(),
		errorBlob.GetAddressOf()
	);

	if (errorBlob != nullptr)
	{
		// todo: Move to Logger
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(
		mDX12Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)
		)
	);

	return true;
}

bool DX12Renderer::CreateShadersAndInputLayout() {
	HRESULT hr = S_OK;

	mvsByteCode = DX12RendererHelper::CompileShader(L"Source\\Resources\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
	mpsByteCode = DX12RendererHelper::CompileShader(L"Source\\Resources\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
		  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return true;
}

bool DX12Renderer::CreatePipelineStateObject() {
	// describe the pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;

	// build the pso
	ThrowIfFailed(
		mDX12Device->CreateGraphicsPipelineState(
			&psoDesc, 
			IID_PPV_ARGS(&mPipelineStateObject)
		)
	);

	return true;
}

bool DX12Renderer::SetupDebugPipeline(
	uint32_t debugSystemMaxCharacters,
	uint32_t fontAtlasIndex
) {
	if (!CreateDebugConstantBufferDescriptors(debugSystemMaxCharacters, fontAtlasIndex)) { return false; }
	if (!CreateDebugRootSignature()) { return false; }
	if (!CreateDebugShadersAndInputLayout()) { return false; }
	if (!CreateDebugPipelineStateObject()) { return false; }

	return true;
}

bool DX12Renderer::CreateDebugConstantBufferDescriptors(uint32_t debugSystemMaxCharacters, uint32_t fontAtlasIndex) {
	uint32_t alignedSizeOfPerPassCb = DX12RendererHelper::CalculateAlignedConstantBufferByteSize(sizeof(DX12DebugSystemPerPassConstants));

	// Compute the global total of descriptors across all frames
	// (1 for SRV) * NoFrames + 1 for Font Atlaas
	UINT numberOfDescriptors = (1 * mNumberOfFrameResources) + 1;

	// Describe the CBV descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numberOfDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	// Create the CBV descriptor heap
	ThrowIfFailed(
		mDX12Device->CreateDescriptorHeap(
			&cbvHeapDesc,
			IID_PPV_ARGS(&mDebugCBVSRVDescriptorHeap)
		)
	);

	return CreateDebugConstantBufferViews(debugSystemMaxCharacters, fontAtlasIndex);
}

bool DX12Renderer::CreateDebugConstantBufferViews(uint32_t debugSystemMaxCharacters, uint32_t fontAtlasIndex) {
	if (fontAtlasIndex >= mTextures.size()) { return false; }
	// create our srv per frame

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

	srvDesc.Buffer.FirstElement = 0;
	// The structural upper boundary limit of your glyph storage pool
	srvDesc.Buffer.NumElements = debugSystemMaxCharacters;
	// Tell the hardware the exact stride width of a single character vertex element
	srvDesc.Buffer.StructureByteStride = sizeof(DX12DebugSystemPerCharacterData);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// Loop through each frame resource and bake the view into its assigned slot
	for (UINT frameIndex = 0; frameIndex < mNumberOfFrameResources; ++frameIndex)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(mDebugCBVSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(frameIndex, mCbvSrvUavDescriptorSize);

		// Grab the raw GPU resource pointer from the active frame tracking element
		auto perCharacterCbResource = mFrameResources[frameIndex]->mDebugSystemPerCharacterCB.Resource();

		// Instantiate the SRV hardware descriptor directly into the heap slot
		mDX12Device->CreateShaderResourceView(perCharacterCbResource, &srvDesc, cpuHandle);
	}

	// create Font Atlas View

	// 1. Point to the new 4th slot at the very end of the debug heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE debugHeapFontCpuHandle(mDebugCBVSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	debugHeapFontCpuHandle.Offset(mNumberOfFrameResources, mCbvSrvUavDescriptorSize);

	// 2. Set up Font texture view description
	// todo: pass in the Font Atlas index
	DX12Texture& tex = mTextures[fontAtlasIndex];
	ID3D12Resource* resource;
	D3D12_SHADER_RESOURCE_VIEW_DESC fontSrvDesc = {};

	// if unloaded 
	if (!tex.IsLoaded || tex.Resource == nullptr) {
		fontSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Generic pixel format
		fontSrvDesc.Texture2D.MipLevels = 1;
		resource = nullptr;
	}
	else {
		fontSrvDesc.Format = tex.Resource->GetDesc().Format; // Grab format from the DDS file
		fontSrvDesc.Texture2D.MipLevels = tex.Resource->GetDesc().MipLevels;
		resource = tex.Resource.Get();
	}

	// Create the Shader Resource View
	fontSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	fontSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	fontSrvDesc.Texture2D.MostDetailedMip = 0;
	fontSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	mDX12Device->CreateShaderResourceView(resource, &fontSrvDesc, debugHeapFontCpuHandle);

	return true;
}

bool DX12Renderer::CreateDebugRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE slotRootRanges[2];

	// Range 0: The Structured Buffer SRV -> register(t0)
	slotRootRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Range 1: The Font Atlas Texture SRV -> register(t1)
	slotRootRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[3];

	// Parameter 0: Direct Root CBV -> register(b0)
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	// Parameter 1: Descriptor Table pointing to your Structured Buffer range
	rootParameters[1].InitAsDescriptorTable(1, &slotRootRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);

	// Parameter 2: Descriptor Table pointing to your Font Texture range
	rootParameters[2].InitAsDescriptorTable(1, &slotRootRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);

	// Static Sampler for smooth texel mapping interpolation
	std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> samplers;
	DX12RendererHelper::GetStaticSamplers(samplers);

	// index 3 is the linear clamp we want
	CD3DX12_STATIC_SAMPLER_DESC fontSampler = samplers[3];
	// map to register(s0) since by default it's mapped to register 3
	fontSampler.ShaderRegister = 0; 

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		_countof(rootParameters),
		rootParameters,
		1,
		&fontSampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(),
		errorBlob.GetAddressOf()
	);

	if (errorBlob != nullptr)
	{
		// todo: Move to Logger
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(
		mDX12Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mDebugRootSignature)
		)
	);

	return true;
}

bool DX12Renderer::CreateDebugShadersAndInputLayout() {
	HRESULT hr = S_OK;

	mDebugVsByteCode = DX12RendererHelper::CompileShader(
		L"Source\\Resources\\Shaders\\Debug\\debug_vs.hlsl",
		nullptr,
		"VS_Main",
		"vs_5_1"
	);

	mDebugPsByteCode = DX12RendererHelper::CompileShader(
		L"Source\\Resources\\Shaders\\Debug\\debug_ps.hlsl", 
		nullptr,
		"PS_Main",
		"ps_5_1"
	);

	return true;
}

bool DX12Renderer::CreateDebugPipelineStateObject() {
	// describe the debug pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.pRootSignature = mDebugRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mDebugVsByteCode->GetBufferPointer()),
		mDebugVsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mDebugPsByteCode->GetBufferPointer()),
		mDebugPsByteCode->GetBufferSize()
	};

	// disable culling
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// enable blending
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// don't write depth values
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthEnable = FALSE;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;

	// build the pso
	ThrowIfFailed(
		mDX12Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(&mDebugPipelineStateObject)
		)
	);

	return true;
}

void DX12Renderer::LoadGeometry(uint32_t meshID, uint16_t sizeOfVertex, uint32_t vertexBufferByteSize, void* vertices, uint32_t indexBufferByteSize, void* indices) {
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
		mCommandList.Get(),
		vertices,
		vertexBufferByteSize,
		meshResource->VertexBufferUploader
	);

	meshResource->IndexBufferGPU = DX12RendererHelper::CreateDefaultBuffer(
		mDX12Device.Get(),
		mCommandList.Get(), 
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
			mCommandList.Get(),
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
	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
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
