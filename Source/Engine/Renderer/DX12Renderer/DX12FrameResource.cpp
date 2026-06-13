#include "DX12FrameResource.h"

#include "../../../Helper/Helper.h"

DX12FrameResource::DX12FrameResource(ID3D12Device* device, UINT perPassCbCount, UINT numberOfEntities, UINT numberOfMaterials, UINT debugSystemPerPassCBCount, UINT debugSystemMaxCharacters) :
    mPerPassCB(device, perPassCbCount, true),
    mPerRenderItemCB(device, numberOfEntities, true),
    mPerMaterialCB(device, numberOfMaterials, true),
    mDebugSystemPerPassCB(device, debugSystemPerPassCBCount, true),
    mDebugSystemPerCharacterCB(device, debugSystemMaxCharacters)
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