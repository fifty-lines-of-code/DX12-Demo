#pragma once

#include "../IRenderer.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <d3d12.h>
#include "d3dx12.h"
#include "DX12 Pipeline Pass/Blur Pass/DX12BlurPipelinePass.h"
#include "DX12 Pipeline Pass/Debug Pass/DX12DebugSystemPipelinePass.h"
#include "DX12 Pipeline Pass/Pipeline Pass Aggregator/DX12PipelinePassAggregator.h"
#include "DX12 Data Structures/DX12PipelineDataStructures.h"
#include "DX12 Pipeline Pass/Opaque Render Pass/DX12OpaqueRenderPipelinePass.h"
#include <fstream>
#include <unordered_map>

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Engine::EngineRenderer::DX12Renderer {

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

		bool Initialize(
			HWND mainHwnd,
			int numberOfFrameResources,
			UINT screenWidth,
			UINT screenHeight
		) override;

		void FinishInitialize() override;
		void Shutdown() override;
		bool LoadTexture(std::wstring& filename, uint32_t id) override;

		bool SetupRenderPipeline(
			uint32_t numberOfEntities,
			uint32_t numberOfMaterials,
			uint32_t numberOfTextures,
			uint32_t sizeOfPerMaterialCBV,
			uint32_t debugSystemPerPassCBCount,
			uint32_t debugSystemMaxCharacters
		);

		bool SetupDebugPipeline(
			uint32_t debugSystemMaxCharacters,
			uint32_t fontAtlasIndex
		);

		bool SetupBlurPipeline();

		void LoadGeometry(
			uint32_t meshID,
			uint16_t sizeOfVertex,
			uint32_t vertexBufferByteSize,
			const void* vertices,
			uint32_t indexBufferByteSize,
			const void* indices
		) override;

		void PrepareForUpdate() override;
		void UpdateOpaqueRenderItemsPerPassCb(
			const void* data, 
			size_t dataSize
		) const override;

		void UpdateOpaqueRenderItemCb(
			uint32_t renderItemIndex,
			const void* data,
			uint32_t perRenderItemCbSize
		) override;

		void UpdatePerMaterialCb(
			uint32_t materialIndex,
			const void* data,
			uint32_t perMaterialCbSize
		) override;

		void UpdateDebugSystemPerPassCb(const void* data) override;
		void UpdateDebugSystemStructuredBuffer(
			uint32_t count,
			const void* data
		);

		void BeginFrame(uint32_t numberOfMaterials) override;
		void Execute(const IPipelinePassExecuteContext& context) override;

		void EndFrame() override;
		void OnResize(UINT width, UINT height) override;

	private:
		DX12FrameResource* mCurrentFrameResource = nullptr;

		D3D12_VIEWPORT mScreenViewport;
		D3D12_RECT mScissorRect;
		UINT64 mCurrentFence = 0;

		// DX12
		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> mDX12Device;
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mInitAndResizeCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mSetupCommandList;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVDescriptorHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffers[DX12RendererConfig::NUMBER_OF_SWAPCHAIN_BUFFERS];
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

		std::vector<std::unique_ptr<DX12FrameResource>> mFrameResources;
		DX12OpaqueRenderPipelinePass mRenderPipelinePass;
		DX12DebugSystemPipelinePass mDebugSystemPipelinePass;
		DX12BlurPipelinePass mBlurPipelinePass;
		DX12PipelinePassAggregator mPiplinePassAggregator;

		// Per Entity Mesh Resource
		std::unordered_map<uint32_t, std::unique_ptr<DX12MeshResource>> mMeshResourceMap;
		// All the Textures
		std::array<DX12Texture, Engine::EngineConfig::EngineConfig::MAX_TEXTURES> mTextures;

		WindowDimensions mWindowDimensions;
		HWND mhMainWnd = nullptr;
		UINT mNumberOfFrameResources = 0;

		// DX12 hardware requirement: Constant buffers must be multiples of 256 bytes.
		UINT				mRtvDescriptorSize = 0;
		UINT				mDsvDescriptorSize = 0;
		UINT				mCbvSrvUavDescriptorSize = 0;
		D3D_DRIVER_TYPE		mD3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT			mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT			mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		UINT				m4xMsaaQuality = 0;      // quality level of 4X MSAA
		UINT				mCurrentBackBufferIndex = 0;
		UINT				mTexturesCbHeapOffset = 0;
		UINT				mCurrentFrameResourceIndex = 0;
		// Set true to use 4X MSAA (§4.1.8).  The default is false.
		bool				m4xMsaaState = false;

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

		void CreateFrameResources(
			uint32_t numberOfEntities,
			uint32_t numberOfMaterials,
			uint32_t debugSystemPerPassCBCount,
			uint32_t debugSystemMaxCharacters
		);

		bool DrawOpaqueRenderItems(
			const DX12PipelinePassExecuteContext& context
		);
		bool DrawDebugSystem(uint32_t numberOfCharacters);
		bool DrawBlurPass();

		void DisposeUploaders();
	};
}