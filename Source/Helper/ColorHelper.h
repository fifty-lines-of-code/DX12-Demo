#pragma once

#include "EngineMath.h"

namespace Engine {
    class ColorHelper {
    public:


        static Vector4 LerpColor(const Vector4& start, const Vector4& end, float factor) {
            return Vector4(
                start.x + factor * (end.x - start.x),
                start.y + factor * (end.y - start.y),
                start.z + factor * (end.z - start.z),
                1.0f
            );
        }
    };
}