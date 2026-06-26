#include "ReflectedCamera.h"

namespace Engine::EngineCamera {

    void ReflectedCamera::UpdateWithMainCameraData(
        const Vector3& mainCamCenter,
        const Vector3& mainCamForward,
        const Vector3& mainCamUp,
        float          fovY,
        float          aspectRatio,
        float          nearZ,
        float          farZ,
        const EngineSimulation::MirrorPlaneQueryResult& mirrorPlanes
    ) {
        mData = {};
        mData.IsValid = false;

        if (mirrorPlanes.Count == 0) {
            return;
        }

        // TODO: Iterate all mirrors and populate an array of ReflectedCameraData.
        //       For now, we extract and process only the first mirror in the array.
        const auto& plane = mirrorPlanes.Planes[0];

        // Load SIMD Registers
        const DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&mainCamCenter.AsXMFLOAT3());
        const DirectX::XMVECTOR camFwd = DirectX::XMLoadFloat3(&mainCamForward.AsXMFLOAT3());
        const DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&mainCamUp.AsXMFLOAT3());
        const DirectX::XMVECTOR planeNormal = DirectX::XMLoadFloat3(&plane.Normal.AsXMFLOAT3());
        const DirectX::XMVECTOR mirrorPos = DirectX::XMLoadFloat3(&plane.Center.AsXMFLOAT3());

        // Half-space validation test
        const DirectX::XMVECTOR mirrorToCamera = DirectX::XMVectorSubtract(camPos, mirrorPos);
        const DirectX::XMVECTOR distVec = DirectX::XMVector3Dot(mirrorToCamera, planeNormal);
        const float dist = DirectX::XMVectorGetX(distVec);

        // Camera too close or behind the mirror plane so cull
        if (dist <= EngineConfig::EngineConfig::MIRROR_PLANE_DISTANCE_EPSILON) {
            return;
        }

        // ANGLE CULL: Skip if camera isn't facing the mirror enough
        // Dot(-Forward, Normal) > 0 means camera faces toward mirror.
        // Threshold of 0.1 approx. 84deg; beyond this, Fresnel dominates and 
        // planar reflection contributes negligible visual value.
        const DirectX::XMVECTOR negCamFwd = DirectX::XMVectorNegate(camFwd);
        const float facingDot = DirectX::XMVectorGetX(
            DirectX::XMVector3Dot(
                negCamFwd, 
                planeNormal
            )
        );

        // Camera facing away or too parallel so cull
        if (facingDot < EngineConfig::EngineConfig::MIRROR_PLANE_ANGLE_EPSILON) {
            return;
        }

        // SIMD Broadcast constants
        const DirectX::XMVECTOR Two = DirectX::XMVectorReplicate(2.0f);

        // 1. Reflect Position: P' = P - (2 * dist) * N
        const DirectX::XMVECTOR twoDist = DirectX::XMVectorScale(distVec, 2.0f);
        const DirectX::XMVECTOR posOffset = DirectX::XMVectorMultiply(twoDist, planeNormal);
        const DirectX::XMVECTOR reflPos = DirectX::XMVectorSubtract(camPos, posOffset);

        // 2. Reflect Forward Vector: F' = F - (2 * Dot(F, N)) * N
        const DirectX::XMVECTOR fwdDotN = DirectX::XMVector3Dot(camFwd, planeNormal);
        const DirectX::XMVECTOR twoTimesFwdDot = DirectX::XMVectorMultiply(Two, fwdDotN);
        const DirectX::XMVECTOR fwdOffset = DirectX::XMVectorMultiply(twoTimesFwdDot, planeNormal);
        const DirectX::XMVECTOR reflFwd = DirectX::XMVectorSubtract(camFwd, fwdOffset);

        // 3. Reflect Up Vector: U' = U - (2 * Dot(U, N)) * N
        const DirectX::XMVECTOR upDotN = DirectX::XMVector3Dot(camUp, planeNormal);
        const DirectX::XMVECTOR twoTimesUpDot = DirectX::XMVectorMultiply(Two, upDotN);
        const DirectX::XMVECTOR upOffset = DirectX::XMVectorMultiply(twoTimesUpDot, planeNormal);
        const DirectX::XMVECTOR reflUp = DirectX::XMVectorSubtract(camUp, upOffset);

        // Normalize reflected direction vectors
        const DirectX::XMVECTOR reflFwdNorm = DirectX::XMVector3Normalize(reflFwd);
        const DirectX::XMVECTOR reflUpNorm = DirectX::XMVector3Normalize(reflUp);

        // 4. Build View Matrix
        const DirectX::XMMATRIX viewXM = DirectX::XMMatrixLookToLH(
            reflPos, 
            reflFwdNorm, 
            reflUpNorm
        );

        // 5. Build Standard Base Projection Matrix
        const DirectX::XMMATRIX projXM = DirectX::XMMatrixPerspectiveFovLH(
            fovY, 
            aspectRatio, 
            nearZ, 
            farZ
        );

        // 6. Calculate View-Projection Matrix
        const DirectX::XMMATRIX vpXM = DirectX::XMMatrixMultiply(viewXM, projXM);

        // Store absolute data payloads
        DirectX::XMStoreFloat4x4(&mData.View.AsXMFLOAT4X4(), viewXM);
        DirectX::XMStoreFloat4x4(&mData.Projection.AsXMFLOAT4X4(), projXM);
        DirectX::XMStoreFloat4x4(&mData.ViewProjection.AsXMFLOAT4X4(), vpXM);
        DirectX::XMStoreFloat3(&mData.Center.AsXMFLOAT3(), reflPos);

        mData.IsValid = true;
    }
}