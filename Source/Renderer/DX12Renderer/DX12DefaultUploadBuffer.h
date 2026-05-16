#pragma once

#include <d3d12.h>
#include "../../d3dx12.h"
#include "../../Helper/Helper.h"
#include "DX12RendererHelper.h"

// Copyright: Frank Luna

template<typename T>
class DX12ConstantBuffersUploadBuffer
{
public:
    DX12ConstantBuffersUploadBuffer(
        ID3D12Device* device,
        UINT elementCount,
        bool isConstantBuffer
    ) : mIsConstantBuffer(isConstantBuffer) {
        mElementByteSize = sizeof(T);

        // Constant buffer elements need to be multiples of 256 bytes.
        // This is because the hardware can only view constant data 
        // at m*256 byte offsets and of n*256 byte lengths. 
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (isConstantBuffer) { mElementByteSize = DX12RendererHelper::CalculateAlignedConstantBufferByteSize(sizeof(T)); }

        auto uploadHepProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount);

        ThrowIfFailed(
            device->CreateCommittedResource(
                &uploadHepProp,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&mUploadBuffer)
            )
        );

        ThrowIfFailed(
            mUploadBuffer->Map(
                0,
                nullptr,
                reinterpret_cast<void**>(&mCpuVirtualAddressHoldingGpuAddress)
            )
        );

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    DX12ConstantBuffersUploadBuffer(const DX12ConstantBuffersUploadBuffer& rhs) = delete;
    DX12ConstantBuffersUploadBuffer& operator=(const DX12ConstantBuffersUploadBuffer& rhs) = delete;
    ~DX12ConstantBuffersUploadBuffer()
    {
        if (mUploadBuffer != nullptr)
            mUploadBuffer->Unmap(0, nullptr);

        mCpuVirtualAddressHoldingGpuAddress = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return mUploadBuffer.Get();
    }

    void CopyData(int elementIndex, const void* data)
    {
        memcpy(&mCpuVirtualAddressHoldingGpuAddress[elementIndex * mElementByteSize], data, sizeof(T));
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
    BYTE* mCpuVirtualAddressHoldingGpuAddress = nullptr;

    UINT mElementByteSize = 0;
    bool mIsConstantBuffer = false;
};