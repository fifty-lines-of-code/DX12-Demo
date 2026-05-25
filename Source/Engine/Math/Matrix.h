#pragma once

namespace Engine {
    struct Matrix4x4 {
        union {
            // Initialize to Identity
            float m[4][4];
            float v[16];
        };

        // We always initialize to identity
        Matrix4x4() noexcept : 
            v{1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f} 
        {}
        Matrix4x4(const Matrix4x4& other) noexcept = default;
        Matrix4x4& operator=(const Matrix4x4& other) noexcept = default;

        inline static Matrix4x4 Identity() noexcept {
            return Matrix4x4();
        }

        const DirectX::XMFLOAT4X4& AsXMFLOAT4X4() const noexcept {
            return *reinterpret_cast<const DirectX::XMFLOAT4X4*>(this);
        }

        DirectX::XMFLOAT4X4& AsXMFLOAT4X4() noexcept {
            return *reinterpret_cast<DirectX::XMFLOAT4X4*>(this);
        }
    };
}
