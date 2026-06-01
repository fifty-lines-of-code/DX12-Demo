#pragma once

#include <cstdint>

namespace Engine {
    class MathHelper {
    public:

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
        static constexpr float Two_Pi = 2.f * MathHelper::Pi;
        static constexpr float Pi_Divide_By_4 = 0.25 * MathHelper::Pi;

        // Copyright: Microsoft's XMConvert*
        static constexpr float ConvertToRadians(float fDegrees) noexcept {
            return fDegrees * (MathHelper::Pi / 180.0f);
        }

        static constexpr float ConvertToDegrees(float fRadians) noexcept {
            return fRadians * (180.0f / MathHelper::Pi);
        }
    };
}