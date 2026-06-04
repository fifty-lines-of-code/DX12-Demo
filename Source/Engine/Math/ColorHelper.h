#pragma once

#include "EngineMath.h"

namespace Engine {
    class ColorHelper {
    public:
        static const Vector4 ColorValley;
        static const Vector4 ColorGrass;
        static const Vector4 ColorRock;
        static const Vector4 ColorSnow;

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

// Definitions for the static const members
const Engine::Vector4 Engine::ColorHelper::ColorValley(0.1f, 0.25f, 0.1f, 1.0f);
const Engine::Vector4 Engine::ColorHelper::ColorGrass(0.25f, 0.55f, 0.2f, 1.0f);
const Engine::Vector4 Engine::ColorHelper::ColorRock(0.45f, 0.38f, 0.32f, 1.0f);
const Engine::Vector4 Engine::ColorHelper::ColorSnow(0.95f, 0.95f, 0.95f, 1.0f);