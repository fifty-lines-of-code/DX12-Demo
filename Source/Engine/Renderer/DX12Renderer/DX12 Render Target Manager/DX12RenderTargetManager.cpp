#include "DX12RenderTargetManager.h"
#include "../d3dx12.h"
#include "../../../../Helper/Helper.h"

namespace Engine::EngineRenderer::DX12Renderer {

    bool DX12RenderTargetManager::Initialize(
        ID3D12Device* device,
        uint32_t width,
        uint32_t height,
        DXGI_FORMAT rtvFormat,
        DXGI_FORMAT dsvFormat
    ) {
        ENGINE_ASSERT(device != nullptr, "Device is null during render target manager init");
        ENGINE_ASSERT(
            width != 0 && height != 0,
            "Width or Height or both are 0 inside render target manager init"
        );
        if (device == nullptr || width == 0 || height == 0) {
            return false;
        }

        mRtvFormat = rtvFormat;
        mDsvFormat = dsvFormat;

        // Create descriptor heaps (survive resize)
        if (!CreateMirrorRtvHeap(device)) { return false; }
        if (!CreateMirrorDsvHeap(device)) { return false; }

        return true;
    }

    void DX12RenderTargetManager::Shutdown() {
        ReleaseMirrorRtvResources();
        ReleaseMirrorDsvResources();
        mMirrorRtvHeap.Reset();
        mMirrorDsvHeap.Reset();
        mRtvFormat = DXGI_FORMAT_UNKNOWN;
        mDsvFormat = DXGI_FORMAT_UNKNOWN;
        mRtvDescriptorSize = 0;
        mDsvDescriptorSize = 0;
    }

    void DX12RenderTargetManager::OnResize(
        ID3D12Device* device,
        uint32_t width,
        uint32_t height
    ) {
        ENGINE_ASSERT(device != nullptr, "DX12 Device is Null, Cannot Resize!!");
        if (device == nullptr) { return; }

        // Release old resources
        ReleaseMirrorRtvResources();
        ReleaseMirrorDsvResources();

        // Recreate with new dimensions
        AllocateMirrorRtvResources(device, width, height);
        CreateMirrorRtvViews(device);

        AllocateMirrorDsvResources(device, width, height);
        CreateMirrorDsvViews(device);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderTargetManager::GetMirrorRtv(
        uint32_t frameIndex
    ) const noexcept {
        if (frameIndex >= DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES) { return {}; }
        return mMirrorRtvEntries[frameIndex].RtvHandle;
    }

    ID3D12Resource* DX12RenderTargetManager::GetMirrorResource(
        uint32_t frameIndex
    ) const noexcept {
        if (frameIndex >= DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES) { return nullptr; }
        return mMirrorRtvEntries[frameIndex].Resource.Get();
    }

    DXGI_FORMAT DX12RenderTargetManager::GetMirrorRtvFormat() const noexcept {
        return mRtvFormat;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderTargetManager::GetMirrorDsv(
        uint32_t frameIndex
    ) const noexcept {
        if (frameIndex >= DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES) { return {}; }
        return mMirrorDsvEntries[frameIndex].DsvHandle;
    }

    DXGI_FORMAT DX12RenderTargetManager::GetMirrorDsvFormat() const noexcept {
        return mDsvFormat;
    }

#pragma region Private - RTV

    bool DX12RenderTargetManager::CreateMirrorRtvHeap(ID3D12Device* device) {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;

        HRESULT hr = device->CreateDescriptorHeap(
            &rtvHeapDesc,
            IID_PPV_ARGS(&mMirrorRtvHeap)
        );
        if (FAILED(hr)) { return false; }

        mRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV
        );

        return true;
    }

    bool DX12RenderTargetManager::AllocateMirrorRtvResources(
        ID3D12Device* device,
        uint32_t width,
        uint32_t height
    ) {
        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

        for (uint32_t frame = 0; frame < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frame) {
            auto& entry = mMirrorRtvEntries[frame];

            D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                mRtvFormat, 
                width, 
                height, 
                1, 
                1
            );
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

            // Initial state: SRV so first barrier transitions to RTV cleanly
            HRESULT hr = device->CreateCommittedResource(
                &defaultHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                nullptr,
                IID_PPV_ARGS(&entry.Resource)
            );
            if (FAILED(hr)) { return false; }

            std::wstring name = L"Mirror RTV: " + std::to_wstring(frame);
            entry.Resource->SetName(name.c_str());
        }

        return true;
    }

    bool DX12RenderTargetManager::CreateMirrorRtvViews(ID3D12Device* device) {
        if (mMirrorRtvHeap == nullptr) { return false; }

        for (uint32_t frame = 0; frame < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frame) {
            auto& entry = mMirrorRtvEntries[frame];

            if (entry.Resource == nullptr) { return false; }

            entry.RtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mMirrorRtvHeap->GetCPUDescriptorHandleForHeapStart(),
                static_cast<INT>(frame),
                static_cast<INT>(mRtvDescriptorSize)
            );

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = mRtvFormat;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = 0;
            rtvDesc.Texture2D.PlaneSlice = 0;

            device->CreateRenderTargetView(
                entry.Resource.Get(), 
                &rtvDesc,
                entry.RtvHandle
            );
        }

        return true;
    }

    void DX12RenderTargetManager::ReleaseMirrorRtvResources() {
        for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++i) {
            mMirrorRtvEntries[i].Resource.Reset();
            // RtvHandle intentionally preserved
        }
    }

#pragma endregion

#pragma region Private - DSV

    bool DX12RenderTargetManager::CreateMirrorDsvHeap(ID3D12Device* device) {
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;

        HRESULT hr = device->CreateDescriptorHeap(
            &dsvHeapDesc,
            IID_PPV_ARGS(&mMirrorDsvHeap)
        );
        if (FAILED(hr)) { return false; }

        mDsvDescriptorSize = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV
        );

        return true;
    }

    bool DX12RenderTargetManager::AllocateMirrorDsvResources(
        ID3D12Device* device,
        uint32_t width,
        uint32_t height
    ) {
        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

        for (uint32_t frame = 0; frame < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frame) {
            auto& entry = mMirrorDsvEntries[frame];

            D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                mDsvFormat, width, height, 1, 1);
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

            // Depth buffers must start in DEPTH_WRITE state
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = mDsvFormat;
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;

            HRESULT hr = device->CreateCommittedResource(
                &defaultHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(&entry.Resource)
            );
            if (FAILED(hr)) { return false; }

            std::wstring name = L"MirrorDSV_Frame" + std::to_wstring(frame);
            entry.Resource->SetName(name.c_str());
        }

        return true;
    }

    bool DX12RenderTargetManager::CreateMirrorDsvViews(ID3D12Device* device) {
        if (mMirrorDsvHeap == nullptr) { return false; }

        for (uint32_t frame = 0; frame < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frame) {
            auto& entry = mMirrorDsvEntries[frame];
            if (entry.Resource == nullptr) { return false; }

            entry.DsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mMirrorDsvHeap->GetCPUDescriptorHandleForHeapStart(),
                static_cast<INT>(frame),
                static_cast<INT>(mDsvDescriptorSize)
            );

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = mDsvFormat;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

            device->CreateDepthStencilView(
                entry.Resource.Get(), 
                &dsvDesc, 
                entry.DsvHandle
            );
        }

        return true;
    }

    void DX12RenderTargetManager::ReleaseMirrorDsvResources() {
        for (uint32_t i = 0; i < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++i) {
            mMirrorDsvEntries[i].Resource.Reset();
            // DsvHandle intentionally preserved
        }
    }

#pragma endregion

}