#pragma once

#include <DirectXMath.h>

struct BasisVectors {
    DirectX::XMFLOAT3 forward = DirectX::XMFLOAT3(0.f, 0.f, 1.f);
    DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(1.f, 0.f, 0.f);
    DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
};

class MathHelper {
public:
    static DirectX::XMFLOAT4X4 Identity4x4()
    {
        static DirectX::XMFLOAT4X4 I(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        return I;
    }

    // Copyright: The legendary Quake III Fast Inverse Square Root function
    static float FastInverseSqrt(float number) {
        float xhalf = 0.5f * number;

        // reinterprets float bytes as an integer
        uint32_t i = *(uint32_t*)&number;

        // The Legendary Magic Number step
        i = 0x5f3759df - (i >> 1);

        // interpret the integer bytes back as a float
        float x = *(float*)&i;

        // One iteration of Newton-Raphson method to reduce error to ~1%
        x = x * (1.5f - xhalf * x * x);

        return x;
    }

    static constexpr float Pi = 3.1415926535f;
    static constexpr float Two_Pi = 2 * MathHelper::Pi;
};