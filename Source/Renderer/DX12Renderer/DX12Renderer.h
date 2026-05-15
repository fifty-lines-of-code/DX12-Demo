#pragma once

#include "../Renderer.h"

#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "../../d3dx12.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;

class Mesh;
struct MeshResource;

class DX12Renderer : public Renderer {

public: 		
	DX12Renderer(int clientWidth, int clientHeight);
	~DX12Renderer();

	bool Initialize(HWND mainHwnd) override;
	bool SetupPipeline(uint32_t numberOfItems, size_t sizeOfEachItem);
	void FinishInitialize();
	void LoadGeometry(const Mesh* const mesh);
	void Shutdown() override;

	void Update(uint32_t meshID, void* data, size_t dataSize) override;
	void BeginFrame() override;
	bool Draw(uint32_t meshID, uint32_t indexCount) override;
	void EndFrame() override;
	void OnResize(UINT newClientWidth, UINT newClientHeight) override;

private:
	bool InitializeDevice();
	void FlushCommandQueue();
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void CreateCommandObjects();
	void CreateSwapChain();
	void BuildDescriptorHeaps();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
	bool CreateRawUploadBufferForConstantBuffer(size_t numberOfItems, size_t sizeOfEachItem);
	bool CreateConstantBufferView(size_t numberOfItems, size_t sizeOfEachItem);
	bool BuildRootSignature();
	bool BuildShadersAndInputLayout();
	bool BuildPipelineStateObject();

private:

	int mClientWidth;
	int mClientHeight;
	HWND mMainHwnd = nullptr;
	static const int SwapChainBufferCount = 2;
	// DX12 hardware requirement: Constant buffers must be multiples of 256 bytes.
	const size_t DX12_CBV_ALIGNMENT = 256;
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// Set true to use 4X MSAA (§4.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA
	UINT64 mCurrentFence = 0;
	UINT mCurrentBackBuffer = 0;

	// DX12
	ComPtr<IDXGIFactory4> mdxgiFactory;
	ComPtr<ID3D12Device> mDX12Device;
	ComPtr<ID3D12Fence> mFence;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mDSVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mCBVDescriptorHeap;
	ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12Resource> mConstantBufferUploadBuffer;
	void* mCpuVirtualAddressHoldingGpuAddressForConstantBuffer = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	ComPtr<ID3D12PipelineState> mPipelineStateObject = nullptr;
	std::vector<std::unique_ptr<MeshResource>> mMeshResources;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
};