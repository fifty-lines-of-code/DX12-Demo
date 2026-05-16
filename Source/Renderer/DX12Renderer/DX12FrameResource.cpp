#include "DX12FrameResource.h"

#include "../../Helper/Helper.h"

DX12FrameResource::DX12FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
{
    ThrowIfFailed(
        device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(mCommandListAllocator.GetAddressOf())
        )
    );

    mPerPassCB = std::make_unique<DX12ConstantBuffersUploadBuffer<DX12PerPassConstants>>(device, passCount, true);
    mPerRenderItemCB = std::make_unique<DX12ConstantBuffersUploadBuffer<DX12RenderItemConstants>>(device, objectCount, true);
}

DX12FrameResource::~DX12FrameResource()
{
    mCommandListAllocator->Reset();
}