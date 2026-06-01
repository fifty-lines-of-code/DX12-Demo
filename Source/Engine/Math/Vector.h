#pragma once

#include <DirectXMath.h>
#include "MathHelper.h"

namespace Engine {
    // Copyright below: Microsoft DirectX::XMFLOAT3
    // 
    // Pretty much copying the definition of XMFLOAT3
    // this will be useful when reinterpreting Vector3 as an XMFLOAT3
    // when sending data to the renderer
    // (assuming DX12 for now)
    // Yes, this is brittle in the sense that if Microsoft changes the underlying 
    // structure of XMFLOAT3, 4, 4x4, then we have to update ours
    // to use methods like XMMAtrixMultiply, which will use XMLoadFloat4x4
    // the only way around it is to write the SIMD operations that happen in
    // XMMatrixMultiply, etc, ourselves

    struct Vector3 {
        float x;
        float y;
        float z;

        Vector3() noexcept : x(0.f), y(0.f), z(0.f) {}
        Vector3(float _x, float _y, float _z) noexcept : x(_x), y(_y), z(_z) {}
        explicit Vector3(_In_reads_(3) const float* pArray) noexcept : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}
        Vector3(const Vector3&) noexcept = default;
        Vector3& operator=(const Vector3&) noexcept = default;

        inline Vector3 operator-(float scalar) const noexcept {
            return Vector3(x - scalar, y - scalar, z - scalar);
        }

        inline Vector3 operator+(float scalar) const noexcept {
            return Vector3(x + scalar, y + scalar, z + scalar);
        }

        inline static Vector3 Zero() noexcept {
            return Vector3(0.f, 0.f, 0.f);
        }

        inline const DirectX::XMFLOAT3& AsXMFLOAT3() const noexcept {
            return *reinterpret_cast<const DirectX::XMFLOAT3*>(this);
        }

        inline DirectX::XMFLOAT3& AsXMFLOAT3() noexcept {
            return *reinterpret_cast<DirectX::XMFLOAT3*>(this);
        }

        inline void Normalize() noexcept {
            float sqLen = (x * x) + (y * y) + (z * z);

            if (sqLen > 0.0001f) {
                float invLen = MathHelper::FastInverseSqrt(sqLen);
                x *= invLen;
                y *= invLen;
                z *= invLen;
            }
        }

        inline void ClampMagnitude(float maxLength) {
            float sqLen = (x * x) + (y * y) + (z * z);

            if (sqLen > (maxLength * maxLength)) {
                float invLen = MathHelper::FastInverseSqrt(sqLen);
                float scale = maxLength * invLen;

                x *= scale;
                y *= scale;
                z *= scale;
            }
        }

        inline void Reset() noexcept {
            x = 0.f;
            y = 0.f;
            z = 0.f;
        }
    };

    struct Vector4 {
        union {
            struct { float x, y, z, w; };
            float v[4];
        };

        Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        Vector4(float _x, float _y, float _z, float _w) noexcept : x(_x), y(_y), z(_z), w(_w) {}
        explicit Vector4(_In_reads_(4) const float* pArray) noexcept : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
        Vector4(const Vector4&) noexcept = default;
        Vector4& operator=(const Vector4&) noexcept = default;

        inline static Vector4 Zero() noexcept {
            return Vector4(0.f, 0.f, 0.f, 0.f);
        }

        const DirectX::XMFLOAT4& AsXMFLOAT4() const noexcept {
            return *reinterpret_cast<const DirectX::XMFLOAT4*>(this);
        }

        DirectX::XMFLOAT4& AsXMFLOAT4() noexcept {
            return *reinterpret_cast<DirectX::XMFLOAT4*>(this);
        }
    };
}