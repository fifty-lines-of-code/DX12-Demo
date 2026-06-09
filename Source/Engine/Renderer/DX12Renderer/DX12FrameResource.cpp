#include "DX12FrameResource.h"

#include "../../../Helper/Helper.h"

DX12FrameResource::DX12FrameResource(ID3D12Device* device, UINT perPassCbCount, UINT numberOfEntities, UINT numberOfMaterials) :
    mPerPassCB(device, perPassCbCount, true),
    mPerRenderItemCB(device, numberOfEntities, true),
    mPerMaterialCB(device, numberOfMaterials, true)
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