#pragma once

#include "../DX12RendererConfig.h"
#include "../DX12RendererHelper.h"

// Copyright: Frank Luna

namespace Engine::EngineRenderer::DX12Renderer {

    // opaque render items
    struct DX12OpaqueRenderItemConstants {
        DirectX::XMFLOAT4X4 World = DX12RendererHelper::Identity4X4();
        uint32_t Padding[48];
    };
    static_assert(
        (sizeof(
            DX12OpaqueRenderItemConstants) % DX12RendererConfig::CONSTANT_BUFFER_SHOULD_BE_MULTIPLES_OF) == 0,
        "Critical: Constant Buffer struct size must be a multiple of 256 bytes.");

    struct DX12OpaqueRenderItemPerSubMeshConstants {
        uint32_t MaterialID = 0;
        uint32_t TextureID = 0;
        uint32_t Padding[62] = { 0, 0 };
    };
    static_assert(
        (sizeof(
            DX12OpaqueRenderItemPerSubMeshConstants) % DX12RendererConfig::CONSTANT_BUFFER_SHOULD_BE_MULTIPLES_OF) == 0,
        "Critical: Constant Buffer struct size must be a multiple of 256 bytes.");

    struct DX12LightData {
        DirectX::XMFLOAT3 Strength;
        float FalloffStart;
        DirectX::XMFLOAT3 Direction;
        float FalloffEnd;
        DirectX::XMFLOAT3 Position;
        float SpotPower;
    };

    struct DX12OpaquePerPassConstants {
        DirectX::XMFLOAT4X4 ViewProjectionTranspose = DX12RendererHelper::Identity4X4();
        DirectX::XMFLOAT4 AmbientLight;
        DirectX::XMFLOAT3 EyePosW;
        float PassPad0;
        DX12LightData Lights[16];
        uint32_t PassPad1[40];
    };
    static_assert(
        (sizeof(
            DX12OpaquePerPassConstants) % DX12RendererConfig::CONSTANT_BUFFER_SHOULD_BE_MULTIPLES_OF) == 0,
        "Critical: Constant Buffer struct size must be a multiple of 256 bytes.");

    // per material constants
    struct DX12PerMaterialConstants {
        DirectX::XMFLOAT4 DiffuseAlbedo;
        DirectX::XMFLOAT3 FresnelR0;
        float Roughness = 0.25f;
        DirectX::XMFLOAT4X4 MatTransform;
    };

    // debug system constants
    struct DX12DebugSystemPerPassConstants {
        float WindowWidth;
        float WindowHeight;
        float QuadWidth;
        float QuadHeight;
        float UV_CellWidth;
        float UV_CellHeight;
        float Padding[2];
    };

    struct DX12DebugSystemPerCharacterData {
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT2 Position;
        uint32_t Ascii;
        uint32_t Padding0;
    };

    // blur compute constants
    struct DX12BlurComputeConstants {
        DirectX::XMFLOAT2 ScreenSize = { 1.f, 1.f };
        DirectX::XMFLOAT2 BlurDirection = { 1.f, 0.f };
        int32_t BlurRadius = 5;
        float OneOverTwoSigmaSq = 0.f;
        DirectX::XMFLOAT2 Padding0 = { 0.f, 0.f };

        // when you update this, make sure to update the Blur Compute shader
    };

}