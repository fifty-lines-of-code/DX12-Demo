#include "ReflectedCamera.h"

#include "../../../Helper/Logger.h"

namespace Engine::EngineCamera {

    void ReflectedCamera::UpdateWithMainCameraData(
        const Vector3& mainCamCenter,
        const Vector3& mainCamForward,
        const Vector3& mainCamUp,
        float          fovY,
        float          aspectRatio,
        float          nearPlane,
        float          farPlane,
        const EngineSimulation::MirrorPlaneQueryResult& mirrorPlanes
    ) noexcept
    {
        mData = {};
        mData.IsValid = false;

        if (mirrorPlanes.Count == 0) {
            return;
        }

        // TODO: Iterate all mirrors and populate an array of ReflectedCameraData.
        //       For now, extract and process only the first mirror in the array.
        const auto& plane = mirrorPlanes.Planes[0];

        // =========================================================================
        // SIMD REGISTER LOADING (Load once, reuse throughout)
        // =========================================================================
        const DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&mainCamCenter.AsXMFLOAT3());
        const DirectX::XMVECTOR camFwd = DirectX::XMLoadFloat3(&mainCamForward.AsXMFLOAT3());
        const DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&mainCamUp.AsXMFLOAT3());
        const DirectX::XMVECTOR planeNormal = DirectX::XMLoadFloat3(&plane.Normal.AsXMFLOAT3());
        const DirectX::XMVECTOR mirrorPos = DirectX::XMLoadFloat3(&plane.Center.AsXMFLOAT3());

        // =========================================================================
        // HALF-SPACE VALIDATION TEST
        // Geometric signed distance: Dot(CameraPos - MirrorPos, Normal)
        // Positive = camera in front of mirror (valid reflection side)
        // =========================================================================
        const DirectX::XMVECTOR mirrorToCamera = DirectX::XMVectorSubtract(camPos, mirrorPos);
        const DirectX::XMVECTOR distVec = DirectX::XMVector3Dot(mirrorToCamera, planeNormal);
        const float dist = DirectX::XMVectorGetX(distVec);

        // Camera too close or behind the mirror plane -> cull
        if (dist <= EngineConfig::EngineConfig::MIRROR_PLANE_DISTANCE_EPSILON) {
            return;
        }

        // =========================================================================
        // ANGLE CULL
        // Dot(-Forward, Normal) measures how directly camera faces the mirror.
        // After negation: positive = facing toward mirror, negative = facing away.
        // Threshold prevents wasted fill-rate at grazing angles where Fresnel
        // dominates and planar reflection contributes negligible visual value.
        // =========================================================================
        const DirectX::XMVECTOR negCamFwd = DirectX::XMVectorNegate(camFwd);
        const float facingDot = DirectX::XMVectorGetX(
            DirectX::XMVector3Dot(negCamFwd, planeNormal)
        );

        if (facingDot < EngineConfig::EngineConfig::MIRROR_PLANE_ANGLE_EPSILON) {
            return;
        }

        // =========================================================================
        // 1. COMPUTE REFLECTED CAMERA POSITION DIRECTLY
        // P' = P - 2 * dist * N
        // [OPTIMAL] 3 FLOPs. Reuses dist and planeNormal already in registers.
        // [AVOID]   Never use XMMatrixInverse to extract position from view matrix.
        //           Matrix inversion is ~200+ FLOPs and completely unnecessary when
        //           the reflection inputs are already available.
        // =========================================================================
        const DirectX::XMVECTOR reflPos = DirectX::XMVectorSubtract(
            camPos,
            DirectX::XMVectorMultiply(
                DirectX::XMVectorReplicate(2.0f * dist),
                planeNormal
            )
        );

        // =========================================================================
        // 2. BUILD PLANAR REFLECTION MATRIX (Householder Transform)
        // Solves plane constant d from: ax + by + cz + d = 0
        // d = -Dot(Normal, PointOnPlane)
        // Full affine reflection matrix in row-major DX12 layout:
        //   | 1-2Nx^2   -2NxNy      -2NxNz      0 |
        //   | -2NyNx    1-2Ny^2     -2NyNz      0 |
        //   | -2NzNx    -2NzNy      1-2Nz^2     0 |
        //   | -2dNx     -2dNy       -2dNz       1 |
        // =========================================================================
        const float d = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(planeNormal, mirrorPos));
        const float Nx = DirectX::XMVectorGetX(planeNormal);
        const float Ny = DirectX::XMVectorGetY(planeNormal);
        const float Nz = DirectX::XMVectorGetZ(planeNormal);

        const DirectX::XMMATRIX manualReflect(
            1.0f - 2.0f * Nx * Nx, -2.0f * Nx * Ny, -2.0f * Nx * Nz, 0.0f,
            -2.0f * Ny * Nx, 1.0f - 2.0f * Ny * Ny, -2.0f * Ny * Nz, 0.0f,
            -2.0f * Nz * Nx, -2.0f * Nz * Ny, 1.0f - 2.0f * Nz * Nz, 0.0f,
            -2.0f * d * Nx, -2.0f * d * Ny, -2.0f * d * Nz, 1.0f
        );

        // =========================================================================
        // 3. COMPOSE REFLECTED VIEW MATRIX
        // COORDINATE SPACE REASONING:
        //   - Reflection operates in WORLD SPACE (it inverts world positions)
        //   - View matrix transforms WORLD -> VIEW space
        //   - Correct order: Reflect FIRST (world space), THEN View (world->view)
        //   - In DXMath row-major: Multiply(A, B) applies A then B
        //   - Therefore: Multiply(manualReflect, mainView) = Reflect then View
        // =========================================================================
        const DirectX::XMMATRIX mainViewXM = DirectX::XMMatrixLookToLH(camPos, camFwd, camUp);
        const DirectX::XMMATRIX viewXM = DirectX::XMMatrixMultiply(manualReflect, mainViewXM);

        // Standard projection (identical to main camera)
        const DirectX::XMMATRIX projXM = DirectX::XMMatrixPerspectiveFovLH(
            fovY, aspectRatio, nearPlane, farPlane
        );

        // Pre-multiply VP on CPU -- never per-vertex on GPU
        const DirectX::XMMATRIX vpXM = DirectX::XMMatrixMultiply(viewXM, projXM);

        // =========================================================================
        // 4. STORE RESULTS
        // =========================================================================
        DirectX::XMStoreFloat4x4(&mData.View.AsXMFLOAT4X4(), viewXM);
        DirectX::XMStoreFloat4x4(&mData.Projection.AsXMFLOAT4X4(), projXM);
        DirectX::XMStoreFloat4x4(&mData.ViewProjection.AsXMFLOAT4X4(), vpXM);
        DirectX::XMStoreFloat3(&mData.Center.AsXMFLOAT3(), reflPos);

        mData.IsValid = true;

        DebugPrintCameraData();
    }

    const Matrix4x4& ReflectedCamera::GetViewProjection() const noexcept {
        return mData.ViewProjection;
    }

#pragma region Private

    void ReflectedCamera::DebugPrintCameraData() const noexcept {
#ifdef _DEBUG
        std::wstring reflPosString =
            L"Reflected Camera Pos - X: " +
            std::to_wstring(mData.Center.x) +
            L", Y: " +
            std::to_wstring(mData.Center.y) +
            L", Z: " +
            std::to_wstring(mData.Center.z) +
            L"\n";

        Logger::PRINT(reflPosString);
#endif
    }

#pragma endregion
}