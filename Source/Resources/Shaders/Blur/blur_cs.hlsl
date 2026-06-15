
// Constants in b0 
// Total Size: 32 bytes (8 DWORDs)
cbuffer BlurConstants : register(b0) {
    // --- ROW 1 (16 Bytes / 4 DWORDs) ---
    float2  gScreenSize;        // 8 bytes | Slots X, Y
    float2  gBlurDirection;     // 8 bytes | Slots Z, W

    // --- ROW 2 (16 Bytes / 4 DWORDs) ---
    int     gBlurRadius;        // 4 bytes | Slot X
    float   gOneOverTwoSigmaSq; // 4 bytes | Slot Y
    float2  gPadding0;          // 8 bytes | Slots Z, W
};

// Texture2D because we're reading as an SRV
Texture2D<float4> gReadTexture : register(t0);

// RWTexture2D because we're writing to it as a UAV
RWTexture2D<float4> gWriteTexture : register(u0);

[numthreads(16, 16, 1)]
void CS_Main(uint3 dispatchThreadID : SV_DispatchThreadID) {

    // safety check, if this threads x or y is > screensize, no op
    if (dispatchThreadID.x >= (uint)gScreenSize.x ||
        dispatchThreadID.y >= (uint)gScreenSize.y)
     { return; } 

    // blur algo

    // final color that we'll write
    float4 colorAccumulator = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // useful to calculate the blur
    float totalWeight = 0.001f;

    // loop over the blur radius
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i) {
        
        // Compute coordinate for neighbor
        int2 neighborCoord = dispatchThreadID.xy + (i * gBlurDirection);
        
        // Clamp to edge 
        neighborCoord = max(int2(0, 0), min(neighborCoord, gScreenSize - 1));
        
        // Direct internal cache load
        float4 neighborColor = gReadTexture[neighborCoord];

        // calculate the weight

        float floatI = (float)i;
        // True Gaussian distribution formula: e^(-x^2 / (2*sigma^2))
        float weight = exp(-(floatI * floatI) * gOneOverTwoSigmaSq);
        
        // accumulate the color
        colorAccumulator += neighborColor * weight;

        // accumulate the weight
        totalWeight += weight;
    }

    // write final color to the write texture
    gWriteTexture[dispatchThreadID.xy] = colorAccumulator / totalWeight;
}