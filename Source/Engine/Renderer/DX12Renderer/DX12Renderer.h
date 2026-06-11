#pragma once

#include "../IRenderer.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include "d3dx12.h"
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "DX12 Texture/DX12Texture.h"
#include <dxgi1_4.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <wrl.h>

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;

struct DX12MeshResource;
struct DX12FrameResource;

struct WindowDimensions {
	UINT Width;
	UINT Height;

	WindowDimensions() : WindowDimensions(0, 0) {}
	WindowDimensions(UINT _width, UINT _height) : Width(_width), Height(_height) {}
	~WindowDimensions() {}
};

class DX12Renderer : public IRenderer {

public: 		
	DX12Renderer();
	~DX12Renderer();

	bool Initialize(HWND mainHwnd, int numberOfFrameResources, UINT screenWidth, UINT screenHeight) override;
	void FinishInitialize() override;
	bool LoadTexture(std::string& name, std::wstring& filename, uint32_t id) override;
	bool SetupPipeline(uint32_t numberOfEntities, uint32_t numberOfMaterials, uint32_t sizeOfPerPassCBV, uint32_t sizeOfPerObjectCBV, uint32_t sizeOfPerMaterialCBV, uint32_t numberOfTextures);
	void LoadGeometry(uint32_t meshID, uint16_t sizeOfVertex, uint32_t vertexBufferByteSize, void* vertices, uint32_t indexBufferByteSize, void* indices) override;
	void PrepareForUpdate() override;
	void UpdatePerPassCb(void* data, size_t dataSize) const override;
	void UpdatePerRenderItemCb(uint32_t renderItemIndex, void* data, uint32_t perRenderItemCbSize) override;
	void UpdatePerMaterialCb(uint32_t materialIndex, void* data, uint32_t perMaterialCbSize) override;
	void BeginFrame(uint32_t numberOfMaterials) override;
	bool Draw(uint32_t meshID, uint32_t indexCount, uint32_t entityIndex, uint32_t entityCount) override;
	void EndFrame() override;
	void Shutdown() override;
	void OnResize(UINT width, UINT height) override;

private:
	static const int SwapChainBufferCount = 2;

	DX12FrameResource* mCurrentFrameResource = nullptr;

	std::array<DX12Texture, Engine::EngineConfig::EngineConfig::MAX_TEXTURES> mTextures;
	std::unordered_map<uint32_t, std::unique_ptr<DX12MeshResource>> mMeshResourceMap;
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::vector<std::unique_ptr<DX12FrameResource>> mFrameResources;
	UINT64 mCurrentFence = 0;

	// DX12
	ComPtr<IDXGIFactory4> mdxgiFactory;
	ComPtr<ID3D12Device> mDX12Device;
	ComPtr<ID3D12Fence> mFence;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandAllocator> mInitAndResizeCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mDSVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mCBVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mSRVDescriptorHeap;
	ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	ComPtr<ID3D12PipelineState> mPipelineStateObject = nullptr;

	WindowDimensions mWindowDimensions;
	HWND mhMainWnd = nullptr;
	UINT mNumberOfFrameResources = 0;

	// DX12 hardware requirement: Constant buffers must be multiples of 256 bytes.
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// Set true to use 4X MSAA (§4.1.8).  The default is false.
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA
	UINT mCurrentBackBuffer = 0;
	UINT mPerEntityCbHeapOffset = 0;
	UINT mPerMaterialCbHeapOffset = 0;
	UINT mCurrentFrameResourceIndex = 0;
	bool      m4xMsaaState = false;    // 4X MSAA enabled

private:
	bool InitializeDevice();
	void FlushCommandQueue();
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvDsvDescriptorHeaps();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
	void CreateFrameResources(uint32_t numberOfEntities, uint32_t numberOfMaterials);
	bool CreateConstantBufferDescriptor(uint32_t numberOfEntities, uint32_t numberOfMaterials, uint32_t sizeOPerPassCb, uint32_t sizeOfPerObjectCb, uint32_t sizeOfPerMaterialCb);
	bool CreateConstantBufferViews(uint32_t numberOfEntities, uint32_t numberOfMaterials, uint32_t alignedSizeOPerPassCb, uint32_t alignedSizeOfPerObjectCB, uint32_t alignedSizeOfPerMaterialCb);
	bool CreateTextureDescriptor(uint32_t numberOfTextures);
	bool CreateTextureResourceViews(uint32_t numberOfTextures);
	bool CreateRootSignature(uint32_t numberOfMaterials, uint32_t numberOfTextures);
	bool CreateShadersAndInputLayout();
	bool CreatePipelineStateObject();
	void DisposeUploaders();
};