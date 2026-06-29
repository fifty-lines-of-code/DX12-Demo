//***************************************************************************************
// mirror_ps.hlsl - Planar reflection pixel shader with projected UVs and full lighting
//***************************************************************************************

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#ifndef NUM_MATERIALS
    #define NUM_MATERIALS 4
#endif

#ifndef NUM_TEXTURES
    #define NUM_TEXTURES 256
#endif

#include "LightingUtil.hlsl"

struct cbMaterial {
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float  gRoughness;
    float4x4 gMatTransform;
};

struct VertexIn {
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
};

struct VertexOut {
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC    : TEXCOORD;
};

cbuffer cbPerObject : register(b0) {
    float4x4 World;
    uint Padding[12];
};

cbuffer cbPerSubMesh : register(b1) {
    uint gMaterialIndex;
    uint gTextureIndex;
    uint gSubMeshPad[15];
};

cbuffer cbPerPass : register(b2) {
    float4x4 ViewProjTranspose;
    float4x4 ReflectedViewProjTranspose;
    float4 AmbientLight;
    float3 EyePosW;
    float PassPad0;
    Light Lights[MaxLights];
    uint4 PassPad1[24];
};

ConstantBuffer<cbMaterial> gMaterials[NUM_MATERIALS] : register(b3);

Texture2D gTextures[NUM_TEXTURES] : register(t0);

SamplerState gsamPointWrap                  : register(s0);
SamplerState gsamPointClamp                 : register(s1);
SamplerState gsamLinearWrap                 : register(s2);
SamplerState gsamLinearClamp                : register(s3);
SamplerState gsamAnisotropicWrap            : register(s4);
SamplerState gsamAnisotropicClamp           : register(s5);
SamplerState gsamMirrorAnisotropicClamp     : register(s6);

float4 Mirror_PS(VertexOut pin) : SV_Target
{
    uint matIdx = min(gMaterialIndex, NUM_MATERIALS - 1);
    cbMaterial matData = gMaterials[matIdx];

    float4 projPos = mul(float4(pin.PosW, 1.0f), ReflectedViewProjTranspose);
    
    // Guard against behind-camera pixels
    if (projPos.w <= 0.0f) discard; 

    float2 mirrorUV = projPos.xy / projPos.w;
    mirrorUV = mirrorUV * 0.5f + 0.5f;
    mirrorUV.y = 1.0f - mirrorUV.y;

    // Use the BORDER sampler to prevent stretching/smearing at edges
    float4 diffuseAlbedo = gTextures[gTextureIndex].Sample(gsamMirrorAnisotropicClamp, mirrorUV)
                         * matData.gDiffuseAlbedo;

    // Lighting - identical to opaque PS, uses MAIN camera EyePosW
    pin.NormalW = normalize(pin.NormalW);

    float3 toEyeW = normalize(EyePosW - pin.PosW);

    float4 ambient = AmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - matData.gRoughness;

    Material mat = { diffuseAlbedo, matData.gFresnelR0, shininess };

    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);

    float4 directLight = ComputeLighting(Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    litColor.a = matData.gDiffuseAlbedo.a;

    return litColor;
}