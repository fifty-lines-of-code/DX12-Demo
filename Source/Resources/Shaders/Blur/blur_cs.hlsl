
// Constants in b0 
// Total Size: 32 bytes (8 DWORDs)
cbuffer BlurConstants : register(b0) {
    // --- ROW 1 (16 Bytes / 4 DWORDs) ---
    float2  gScreenSize;      // 8 bytes | Slots X, Y
    float2  gBlurDirection;   // 8 bytes | Slots Z, W

    // --- ROW 2 (16 Bytes / 4 DWORDs) ---
    int    gBlurRadius;      // 4 bytes | Slot X
    float3  gPadding0;        // 8 bytes | Slots Y, Z, W
};

Texture2D<float4> gReadTexture : register(t0);
RWTexture2D<float4> gWriteTexture : register(u0);
SamplerState gClampSampler : register(s0);

[numthreads(16, 16, 1)]
void CS_Main(uint3 dispatchThreadID : SV_DispatchThreadID) {

    // safety check, if this threads x or y is > screensize, no op
    if (dispatchThreadID.x >= (uint)gScreenSize.x ||
        dispatchThreadID.y >= (uint)gScreenSize.y)
     { return; } 

    // blur algo
    
    // calculate center of pixel in UV coordinates (0 to 1)
    float2 pixelCenterInUV = (float2(dispatchThreadID.xy) + 0.5f) / gScreenSize;
    float4 colorAccumulator = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float totalWeight = 0.001f;


    // Standard deviation controls the spread
    float sigma = float(gBlurRadius) / 1.5f;
    float twoSigmaSq = 2.0f * sigma * sigma;
    float oneOverTwoSigmaSq = 1.0 / twoSigmaSq;

    // loop over the blur radius
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i) {
        // calculate the weight
        float floatI = (float)i;

        // True Gaussian distribution formula: e^(-x^2 / (2*sigma^2))
        float weight = exp(-(floatI * floatI) * oneOverTwoSigmaSq);
        
        // calculate UV offset for this pixel
        float2 uvOffset = (floatI / gScreenSize) * gBlurDirection;
        
        float4 neighborColor = gReadTexture.SampleLevel(
            gClampSampler,
            pixelCenterInUV + uvOffset,
            0
        );
        
        // accumulate the color
        colorAccumulator += neighborColor * weight;

        // accumulate the weight
        totalWeight += weight;
    }

    // write final color to the write texture
    colorAccumulator /= totalWeight;
    colorAccumulator.w = 1.0;
    gWriteTexture[dispatchThreadID.xy] = colorAccumulator;
}