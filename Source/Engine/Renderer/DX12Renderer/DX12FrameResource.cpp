#include "DX12FrameResource.h"

#include "../../../Helper/Helper.h"

DX12FrameResource::DX12FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount) :
    mPerPassCB(device, passCount, true),
    mPerRenderItemCB(device, objectCount, true)
{
    ThrowIfFailed(
        device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(mCommandListAllocator.GetAddressOf())
        )
    );
}

DX12FrameResource::~DX12FrameResource()
{
    mCommandListAllocator->Reset();
}