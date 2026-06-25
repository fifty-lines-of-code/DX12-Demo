//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

// Defaults for number of lights.
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
    #define NUM_TEXTURES 128
#endif

#include "LightingUtil.hlsl"

// data structures
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

// buffers passed in per vertex and pixel
cbuffer cbPerObject : register(b0) {
    float4x4 World;
    uint Padding[12];
};

cbuffer cbPerSubMesh : register(b1) {
    uint gMaterialIndex;
    uint gTextureIndex;
    uint gSubMeshPad[15];
};

cbuffer cbPerPass: register(b2) {
    float4x4 ViewProjTranspose;
    float4 AmbientLight;
    float3 EyePosW;
    float PassPad0;
    Light Lights[MaxLights]; // MaxLights is defined inside LightingUtil.hlsl
    uint4 PassPad1[40];
};

ConstantBuffer<cbMaterial> gMaterials[NUM_MATERIALS] : register(b3);

Texture2D gTextures[NUM_TEXTURES] : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// vertex shader
VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space
    float4 posW = mul(float4(vin.PosL, 1.0f), World);
    vout.PosW = posW.xyz;
    
    // Transform to homogeneous clip space
    vout.PosH = mul(posW, ViewProjTranspose);
    
    // Transform normals to world space
    vout.NormalW = mul(vin.NormalL, (float3x3)World);

    // transform texcoords once we want to animate some texture
    vout.TexC = vin.TexC;
    
    return vout;
}

// pixel shader
float4 PS(VertexOut pin) : SV_Target
{
    // get material data using material index
    // todo: find a way to remove this if condition
    int index = 0;
    if (gMaterialIndex < NUM_MATERIALS) {
        index = gMaterialIndex;
    }
    cbMaterial matData = gMaterials[index];

    // calculate diffuse albedo by texture sample * matData.gDiffuseAlbedo 
    // if we have a valid texture id
    // todo: Find another way to find the texture ID without the if

    float4 diffuseAlbedo = matData.gDiffuseAlbedo;
    if (gTextureIndex < 3) {
        diffuseAlbedo = gTextures[gTextureIndex].Sample(gsamAnisotropicWrap, pin.TexC) * diffuseAlbedo;
    }
 
    // Interpolating a normal can unnormalize it, so renormalize it
    pin.NormalW = normalize(pin.NormalW);
    
    // Vector from point being lit to eye
    float3 toEyeW = normalize(EyePosW - pin.PosW); 

    // Indirect ambient lighting computation
    float4 ambient = AmbientLight * diffuseAlbedo;

    // Convert material roughness up to shininess for Luna's blinn-phong utility
    const float shininess = 1.0f - matData.gRoughness;

    // Map specific unpacked matData members into Luna's lighting engine struct
    Material mat = { diffuseAlbedo, matData.gFresnelR0, shininess };
    
    // Shadow factor placeholder (1.0f means completely unshadowed)
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    
    // Pass the correct 'Lights' array variable to Luna's core function
    float4 directLight = ComputeLighting(Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material
    litColor.a = matData.gDiffuseAlbedo.a;

    return litColor;
}