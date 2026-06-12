#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "DX12DefaultUploadBuffer.h"

// Copyright: Frank Luna

struct DX12PerRenderItemConstants
{
    DirectX::XMFLOAT4X4 World = DX12RendererHelper::Identity4X4();
    uint32_t MaterialID = 0;
    uint32_t TextureID = 0;
};

struct DX12LightData {
    DirectX::XMFLOAT3 Strength;
    float FalloffStart;
    DirectX::XMFLOAT3 Direction;
    float FalloffEnd;
    DirectX::XMFLOAT3 Position;
    float SpotPower;
};

struct DX12PerPassConstants
{
    DirectX::XMFLOAT4X4 ViewProjectionTranspose = DX12RendererHelper::Identity4X4();
    DirectX::XMFLOAT4 AmbientLight; 
    DirectX::XMFLOAT3 EyePosW; 
    float PassPad0;
    DX12LightData Lights[16];
};

struct DX12PerMaterialConstants
{
    DirectX::XMFLOAT4 DiffuseAlbedo;
    DirectX::XMFLOAT3 FresnelR0;
    float Roughness = 0.25f;
    DirectX::XMFLOAT4X4 MatTransform;
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct DX12FrameResource
{
public:

    DX12FrameResource(ID3D12Device* device, UINT perPassCbCount, UINT numberOfEntities, UINT numberOfMaterials);
    DX12FrameResource(const DX12FrameResource& rhs) = delete;
    DX12FrameResource& operator=(const DX12FrameResource& rhs) = delete;
    ~DX12FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandListAllocator;

    // We cannot update a cbuffer until the GPU is done processing the commands that reference it. 
    // So each frame needs their own cbuffers.
    DX12ConstantBuffersUploadBuffer<DX12PerPassConstants> mPerPassCB;
    DX12ConstantBuffersUploadBuffer<DX12PerRenderItemConstants> mPerRenderItemCB;
    DX12ConstantBuffersUploadBuffer<DX12PerMaterialConstants> mPerMaterialCB;
    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 mFenceValue = 0;
};