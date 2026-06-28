#pragma once

#include <array>
#include <cstdint>
#include <d3d12.h>
#include "../DX12RendererConfig.h"
#include <wrl.h>

namespace Engine::EngineRenderer::DX12Renderer {

    class DX12RenderTargetManager {
    public:
        DX12RenderTargetManager() noexcept = default;
        ~DX12RenderTargetManager() = default;

        DX12RenderTargetManager(const DX12RenderTargetManager&) = delete;
        DX12RenderTargetManager& operator=(const DX12RenderTargetManager&) = delete;
        DX12RenderTargetManager(DX12RenderTargetManager&&) = delete;
        DX12RenderTargetManager& operator=(DX12RenderTargetManager&&) = delete;

        bool Initialize(
            ID3D12Device* device,
            uint32_t width,
            uint32_t height,
            DXGI_FORMAT rtvFormat,
            DXGI_FORMAT dsvFormat
        );

        void Shutdown();

        void OnResize(
            ID3D12Device* device,
            uint32_t width,
            uint32_t height
        );

        D3D12_CPU_DESCRIPTOR_HANDLE GetMirrorRtv(
            uint32_t frameIndex
        ) const noexcept;
        ID3D12Resource* GetMirrorResource(
            uint32_t frameIndex
        ) const noexcept;
        DXGI_FORMAT GetMirrorRtvFormat() const noexcept;

        D3D12_CPU_DESCRIPTOR_HANDLE GetMirrorDsv(
            uint32_t frameIndex
        ) const noexcept;
        DXGI_FORMAT GetMirrorDsvFormat() const noexcept;

    private:
        struct MirrorRtvEntry {
            Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
            D3D12_CPU_DESCRIPTOR_HANDLE            RtvHandle = {};
        };

        struct MirrorDsvEntry {
            Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
            D3D12_CPU_DESCRIPTOR_HANDLE            DsvHandle = {};
        };

        std::array<MirrorRtvEntry, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES> mMirrorRtvEntries = {};
        std::array<MirrorDsvEntry, DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES> mMirrorDsvEntries = {};

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mMirrorRtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mMirrorDsvHeap;

        DXGI_FORMAT mRtvFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT mDsvFormat = DXGI_FORMAT_UNKNOWN;
        uint32_t mRtvDescriptorSize = 0;
        uint32_t mDsvDescriptorSize = 0;

    private:
        // RTV methods
        bool CreateMirrorRtvHeap(ID3D12Device* device);
        bool AllocateMirrorRtvResources(ID3D12Device* device, uint32_t width, uint32_t height);
        bool CreateMirrorRtvViews(ID3D12Device* device);
        void ReleaseMirrorRtvResources();

        // DSV methods
        bool CreateMirrorDsvHeap(ID3D12Device* device);
        bool AllocateMirrorDsvResources(ID3D12Device* device, uint32_t width, uint32_t height);
        bool CreateMirrorDsvViews(ID3D12Device* device);
        void ReleaseMirrorDsvResources();
    };

}