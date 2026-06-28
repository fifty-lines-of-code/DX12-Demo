#pragma once

#include <array>
#include <cstdint>
#include "../d3dx12.h"
#include "../DX12RendererConfig.h"
#include <wrl/client.h>

namespace Engine::EngineRenderer::DX12Renderer {

    struct MaterialCbvInitArgs {
        uint32_t FrameIndex;
        uint32_t MaterialIndex;
        D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;
        uint32_t SizeInBytes;
    };

    struct TextureSrvInitArgs {
        uint32_t TextureIndex;
        ID3D12Resource* Resource;
        D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
    };

    class DX12DescriptorManager {
    public:
        DX12DescriptorManager() noexcept = default;
        ~DX12DescriptorManager() = default;

        DX12DescriptorManager(const DX12DescriptorManager&) = delete;
        DX12DescriptorManager& operator=(const DX12DescriptorManager&) = delete;
        DX12DescriptorManager(DX12DescriptorManager&&) = delete;
        DX12DescriptorManager& operator=(DX12DescriptorManager&&) = delete;

        // Creates heap AND populates all material CBVs and texture SRVs in one call.
        // Mirror SRVs are NOT created here (resources don't exist yet at init time).
        bool Initialize(
            ID3D12Device* device,
            uint32_t numMaterials,
            const MaterialCbvInitArgs* materialCbvs,
            uint32_t numMaterialCbvs,
            const TextureSrvInitArgs* textureSrvs,
            uint32_t numTextureSrvs
        );

        void Shutdown();

        // Mirror SRVs created/recreated separately because underlying
        // RTV resources change on every window resize.
        void CreateMirrorRtvSrv(
            ID3D12Device* device,
            uint32_t frameIndex,
            ID3D12Resource* rtvResource
        );

        ID3D12DescriptorHeap* GetSharedHeap() const noexcept;

        CD3DX12_GPU_DESCRIPTOR_HANDLE GetMaterialsDescriptorHandle(
            uint32_t frameIndex
        ) const noexcept;

        CD3DX12_GPU_DESCRIPTOR_HANDLE GetTexturesDescriptorHandle() const noexcept;

    private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSharedHeap;

        uint32_t mDescriptorSize = 0;
        uint32_t mNumMaterials = 0;
        uint32_t mNumTextures = 0;

        std::array<CD3DX12_GPU_DESCRIPTOR_HANDLE, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES> mMaterialsGPUDescriptorHandles;

        CD3DX12_GPU_DESCRIPTOR_HANDLE mTexturesGPUDescriptorHandle;

        uint32_t mTextureSrvBaseIndex = 0;
        uint32_t mMirrorSrvBaseIndex = 0;

    private:
        void CreateMaterialCbv(
            ID3D12Device* device,
            uint32_t frameIndex,
            uint32_t materialIndex,
            D3D12_GPU_VIRTUAL_ADDRESS bufferLocation,
            uint32_t sizeInBytes
        );

        void CreateTextureSrv(
            ID3D12Device* device,
            uint32_t textureIndex,
            ID3D12Resource* resource,
            const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc
        );
    };
}