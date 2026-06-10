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

#include "LightingUtil.hlsl"

struct cbMaterial {
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float  Roughness;
    float4x4 MatTransform;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 World;
    uint MaterialIndex;
};

cbuffer cbPerPass: register(b1) 
{
    float4x4 ViewProjTranspose;
    float4 AmbientLight;
    float3 EyePosW;
    float PassPad0;
    Light Lights[MaxLights]; // MaxLights is defined natively inside LightingUtil.hlsl
};

ConstantBuffer<cbMaterial> Materials[3] : register(b2);

struct VertexIn
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut
{
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

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
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Interpolating a normal can unnormalize it, so renormalize it
    pin.NormalW = normalize(pin.NormalW);
    
    // Vector from point being lit to eye
    float3 toEyeW = normalize(EyePosW - pin.PosW); 

    // get material data using material index
    cbMaterial matData = Materials[MaterialIndex];

    // Indirect ambient lighting computation
    float4 ambient = AmbientLight * matData.DiffuseAlbedo;

    // Convert material roughness up to shininess for Luna's blinn-phong utility
    const float shininess = 1.0f - matData.Roughness;

    // Map specific unpacked matData members into Luna's lighting engine struct
    Material mat = { matData.DiffuseAlbedo, matData.FresnelR0, shininess };
    
    // Shadow factor placeholder (1.0f means completely unshadowed)
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    
    // FIX: Pass the correct 'Lights' array variable to Luna's core function
    float4 directLight = ComputeLighting(Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material
    litColor.a = matData.DiffuseAlbedo.a;

    return litColor;
}