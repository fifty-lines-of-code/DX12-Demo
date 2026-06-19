#pragma once

#include "../d3dx12.h"
#include "../../../../Helper/Helper.h"
#include "../DX12RendererHelper.h"

// Copyright: Frank Luna

template<typename T>
class DX12StructuredUploadBuffer
{
public:
    DX12StructuredUploadBuffer(
        ID3D12Device* device,
        UINT elementCount
    ) : mElementCount(elementCount)
    {
        mElementByteSize = sizeof(T);

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

    DX12StructuredUploadBuffer(const DX12StructuredUploadBuffer& rhs) = delete;
    DX12StructuredUploadBuffer& operator=(const DX12StructuredUploadBuffer& rhs) = delete;
    ~DX12StructuredUploadBuffer()
    {
        if (mUploadBuffer != nullptr)
            mUploadBuffer->Unmap(0, nullptr);

        mCpuVirtualAddressHoldingGpuAddress = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return mUploadBuffer.Get();
    }

    void CopyData(uint32_t count, const void* data)
    {
        if (count > mElementCount) {
            count = mElementCount;
        }

        memcpy(
            mCpuVirtualAddressHoldingGpuAddress,
            data,
            count * sizeof(T)
        );
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
    BYTE* mCpuVirtualAddressHoldingGpuAddress = nullptr;
    UINT mElementCount;
    UINT mElementByteSize = 0;
};