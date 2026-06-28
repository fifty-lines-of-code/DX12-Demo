#pragma once

#include "../DX12 Upload Buffers/DX12DefaultUploadBuffer.h"
#include "../DX12 Upload Buffers/DX12StructuredUploadBuffer.h"
#include "../DX12 Data Structures/DX12ResourceDataStructures.h"

// Copyright: Frank Luna

// Stores the resources needed for the CPU to build the command lists
// for a frame.  

namespace Engine::EngineRenderer::DX12Renderer {

    struct DX12FrameResource {
    public:

        DX12FrameResource(
            ID3D12Device* device,
            UINT perPassCbCount,
            UINT numberOfEntities,
            UINT maxSubMeshesPerEntity,
            UINT numberOfMaterials,
            UINT debugSystemPerPassCBCount,
            UINT debugSystemMaxCharacters
        );
        ~DX12FrameResource();

        DX12FrameResource(const DX12FrameResource& rhs) = delete;
        DX12FrameResource& operator=(const DX12FrameResource& rhs) = delete;

        // We cannot reset the allocator until the GPU is done processing the commands.
        // So each frame needs their own allocator.
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;

        // We cannot update a cbuffer until the GPU is done processing the commands that reference it. 
        // So each frame needs their own cbuffers.
        DX12ConstantBuffersUploadBuffer<DX12OpaquePerPassConstants> mOpaquePassRenderItemsPerPassCB;
        DX12ConstantBuffersUploadBuffer<DX12OpaquePerPassConstants> mMirrorPassRenderItemsPerPassCB;
        DX12ConstantBuffersUploadBuffer<DX12OpaqueRenderItemConstants> mOpaqueRenderItemCB;
        DX12ConstantBuffersUploadBuffer<DX12OpaqueRenderItemPerSubMeshConstants> mOpaqueRenderItemPerSubMeshCB;
        DX12ConstantBuffersUploadBuffer<DX12PerMaterialConstants> mPerMaterialCB;
        DX12ConstantBuffersUploadBuffer<DX12DebugSystemPerPassConstants> mDebugSystemPerPassCB;
        DX12StructuredUploadBuffer<DX12DebugSystemPerCharacterData> mDebugSystemPerCharacterCB;
        // Fence value to mark commands up to this fence point.  This lets us
        // check if these frame resources are still in use by the GPU.
        UINT64 mFenceValue = 0;
    };
}