#include "DX12DescriptorManager.h"

#include "../../../../Helper/Helper.h"

namespace Engine::EngineRenderer::DX12Renderer {

    bool DX12DescriptorManager::Initialize(
        ID3D12Device* device,
        uint32_t numMaterials,
        const MaterialCbvInitArgs* materialCbvs,
        uint32_t numMaterialCbvs,
        const TextureSrvInitArgs* textureSrvs,
        uint32_t numTextureSrvs
    ) {
        ENGINE_ASSERT(device != nullptr, "Device is null during descriptor manager init");
        if (device == nullptr) { return false; }

        // Derive material count from flat array size
        // materialCbvs contains numMaterials * numFrames entries
        mNumMaterials = numMaterials;
        mNumTextures = numTextureSrvs;

        // Precompute layout offsets
        // Layout: [MatCB F0..FN][Tex SRV 0..N][Mirror SRV F0..FN]
        mTextureSrvBaseIndex = mNumMaterials * DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES;
        mMirrorSrvBaseIndex = mTextureSrvBaseIndex + mNumTextures;

        uint32_t totalDescriptors = 
            mMirrorSrvBaseIndex + 
            DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES;

        // describe the heap
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = totalDescriptors;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask = 0;

        // create the heap
        HRESULT hr = device->CreateDescriptorHeap(
            &heapDesc, 
            IID_PPV_ARGS(&mSharedHeap)
        );
        if (FAILED(hr)) { return false; }

        // store the descriptor size
        mDescriptorSize = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        );

        // get handle to the heap staet
        auto heapStart = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            mSharedHeap->GetGPUDescriptorHandleForHeapStart()
        );

        // store materials gpu descriptor handles
        for (uint32_t frame = 0; frame < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frame) {
            auto materialsDescriptorHandle = heapStart;
            materialsDescriptorHandle.Offset(
                mNumMaterials * frame,
                mDescriptorSize
            );
            mMaterialsGPUDescriptorHandles[frame] = materialsDescriptorHandle;
        }

        // store the textures gpu descriptor handle
        auto texturesDescriptorHandle = heapStart;
        texturesDescriptorHandle.Offset(
            mTextureSrvBaseIndex,
            mDescriptorSize
        );
        mTexturesGPUDescriptorHandle = texturesDescriptorHandle;

        // Populate all material CBVs
        for (uint32_t i = 0; i < numMaterialCbvs; ++i) {
            const auto& cbv = materialCbvs[i];
            CreateMaterialCbv(
                device,
                cbv.FrameIndex,
                cbv.MaterialIndex,
                cbv.BufferLocation,
                cbv.SizeInBytes
            );
        }

        // Populate all texture SRVs
        for (uint32_t i = 0; i < numTextureSrvs; ++i) {
            const auto& srv = textureSrvs[i];
            CreateTextureSrv(
                device,
                srv.TextureIndex,
                srv.Resource,
                srv.SrvDesc
            );
        }

        return true;
    }

    void DX12DescriptorManager::Shutdown() {
        mSharedHeap.Reset();
        mDescriptorSize = 0;
        mNumMaterials = 0;
        mNumTextures = 0;
        mTexturesGPUDescriptorHandle = {};
        mTextureSrvBaseIndex = 0;
        mMirrorSrvBaseIndex = 0;
    }

    void DX12DescriptorManager::CreateMirrorRtvSrv(
        ID3D12Device* device,
        uint32_t frameIndex,
        ID3D12Resource* rtvResource
    ) {
        ENGINE_ASSERT(device != nullptr, "Device is null creating mirror RTV SRV");
        ENGINE_ASSERT(
            frameIndex < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES,
            "Mirror SRV frame index out of range"
        );
        ENGINE_ASSERT(rtvResource != nullptr, "Mirror RTV resource is null");

        // Mirror SRVs start immediately after texture SRVs in the shared heap
        const uint32_t mirrorSrvBaseIndex = mTextureSrvBaseIndex + mNumTextures;
        uint32_t heapIndex = mirrorSrvBaseIndex + frameIndex;

        auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            mSharedHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(heapIndex),
            static_cast<INT>(mDescriptorSize));

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        device->CreateShaderResourceView(rtvResource, &srvDesc, cpuHandle);
    }

    ID3D12DescriptorHeap* DX12DescriptorManager::GetSharedHeap() const noexcept {
        return mSharedHeap.Get();
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DX12DescriptorManager::GetMaterialsDescriptorHandle(
        uint32_t frameIndex
    ) const noexcept {
        if (frameIndex >= DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES) { return {}; }

        return mMaterialsGPUDescriptorHandles[frameIndex];
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DX12DescriptorManager::GetTexturesDescriptorHandle() const noexcept {
        return mTexturesGPUDescriptorHandle;
    }

#pragma region Private

    void DX12DescriptorManager::CreateMaterialCbv(
        ID3D12Device* device,
        uint32_t frameIndex,
        uint32_t materialIndex,
        D3D12_GPU_VIRTUAL_ADDRESS bufferLocation,
        uint32_t sizeInBytes
    ) {
        ENGINE_ASSERT(device != nullptr, "Device is null creating material CBV");
        ENGINE_ASSERT(
            frameIndex < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES,
            "Material CBV frame index out of range"
        );

        uint32_t heapIndex = (mNumMaterials * frameIndex) + materialIndex;

        auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            mSharedHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(heapIndex),
            static_cast<INT>(mDescriptorSize)
        );

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = bufferLocation;
        cbvDesc.SizeInBytes = sizeInBytes;

        device->CreateConstantBufferView(&cbvDesc, cpuHandle);
    }

    void DX12DescriptorManager::CreateTextureSrv(
        ID3D12Device* device,
        uint32_t textureIndex,
        ID3D12Resource* resource,
        const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc
    ) {
        ENGINE_ASSERT(device != nullptr, "Device is null creating texture SRV");
        ENGINE_ASSERT(textureIndex < mNumTextures, "Texture SRV index out of range");

        uint32_t heapIndex = mTextureSrvBaseIndex + textureIndex;

        auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            mSharedHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(heapIndex),
            static_cast<INT>(mDescriptorSize)
        );

        device->CreateShaderResourceView(resource, &srvDesc, cpuHandle);
    }

#pragma endregion
}