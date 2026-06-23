#include "DX12FrameResource.h"

#include "../../../../Helper/Helper.h"

namespace Engine::EngineRenderer::DX12Renderer {

    DX12FrameResource::DX12FrameResource(
        ID3D12Device* device, 
        UINT perPassCbCount, 
        UINT numberOfEntities,
        UINT maxSubMeshesPerEntity,
        UINT numberOfMaterials,
        UINT debugSystemPerPassCBCount, 
        UINT debugSystemMaxCharacters
    ) :
        mOpaquePerPassCB(device, perPassCbCount, true),
        mOpaqueRenderItemCB(device, numberOfEntities, true),
        mOpaqueRenderItemPerSubMeshCB(device, numberOfEntities * maxSubMeshesPerEntity, true),
        mPerMaterialCB(device, numberOfMaterials, true),
        mDebugSystemPerPassCB(device, debugSystemPerPassCBCount, true),
        mDebugSystemPerCharacterCB(device, debugSystemMaxCharacters)
    {
        ThrowIfFailed(
            device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(mCommandAllocator.GetAddressOf())
            )
        );
    }

    DX12FrameResource::~DX12FrameResource() {
        mCommandAllocator->Reset();
    }
}