#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "DX12DefaultUploadBuffer.h"

// Copyright: Frank Luna

struct DX12RenderItemConstants
{
    DirectX::XMFLOAT4X4 World = DX12RendererHelper::Identity4X4();
};

struct DX12PerPassConstants
{
    DirectX::XMFLOAT4X4 View = DX12RendererHelper::Identity4X4();
    DirectX::XMFLOAT4X4 Proj = DX12RendererHelper::Identity4X4();
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct DX12FrameResource
{
public:

    DX12FrameResource(ID3D12Device* device, UINT perPassCbCount, UINT numberOfEntities);
    DX12FrameResource(const DX12FrameResource& rhs) = delete;
    DX12FrameResource& operator=(const DX12FrameResource& rhs) = delete;
    ~DX12FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandListAllocator;

    // We cannot update a cbuffer until the GPU is done processing the commands that reference it. 
    // So each frame needs their own cbuffers.
    DX12ConstantBuffersUploadBuffer<DX12PerPassConstants> mPerPassCB;
    DX12ConstantBuffersUploadBuffer<DX12RenderItemConstants> mPerRenderItemCB;
    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 mFenceValue = 0;
};