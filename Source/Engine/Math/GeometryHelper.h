#pragma once

#include "EngineMath.h"
#include "Geometry.h"

namespace Engine {

	class GeometryHelper {
    public:
        static void CalculateAABB(
            const AABB& localAABB, 
            const Matrix4x4& worldMatrix, 
            AABB& outWorldAABB) {
            DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(&worldMatrix.AsXMFLOAT4X4());

            const DirectX::XMFLOAT3& min = localAABB.Min.AsXMFLOAT3();
            const DirectX::XMFLOAT3& max = localAABB.Max.AsXMFLOAT3();

            const int numberOfVerticesInACube = 8;

            // Generate all 8 vertices using clean, fast stack-allocated SIMD registers
            DirectX::XMVECTOR aabbVertices[numberOfVerticesInACube] = {
                DirectX::XMVectorSet(min.x, min.y, max.z, 1.0f),
                DirectX::XMVectorSet(max.x, min.y, max.z, 1.0f),
                DirectX::XMVectorSet(max.x, min.y, min.z, 1.0f),
                DirectX::XMVectorSet(min.x, min.y, min.z, 1.0f),
                DirectX::XMVectorSet(min.x, max.y, max.z, 1.0f),
                DirectX::XMVectorSet(max.x, max.y, max.z, 1.0f),
                DirectX::XMVectorSet(max.x, max.y, min.z, 1.0f),
                DirectX::XMVectorSet(min.x, max.y, min.z, 1.0f)
            };

            DirectX::XMVECTOR vTransformed = DirectX::XMVector3TransformCoord(aabbVertices[0], matrix);
            DirectX::XMVECTOR vWorldMin = vTransformed;
            DirectX::XMVECTOR vWorldMax = vTransformed;

            for (int i = 1; i < numberOfVerticesInACube; ++i) {
                vTransformed = DirectX::XMVector3TransformCoord(aabbVertices[i], matrix);
                vWorldMin = DirectX::XMVectorMin(vWorldMin, vTransformed);
                vWorldMax = DirectX::XMVectorMax(vWorldMax, vTransformed);
            }

            DirectX::XMStoreFloat3(&outWorldAABB.Min.AsXMFLOAT3(), vWorldMin);
            DirectX::XMStoreFloat3(&outWorldAABB.Max.AsXMFLOAT3(), vWorldMax);
        }

        static bool AABBIntersect(const AABB& first, const AABB& second) {
            // check x
            if (first.Max.x < second.Min.x ||
                first.Min.x > second.Max.x) {
                return false;
            }

            // check y
            if (first.Max.y < second.Min.y ||
                first.Min.y > second.Max.y) {
                return false;
            }

            // check z
            if (first.Max.z < second.Min.z ||
                first.Min.z > second.Max.z) {
                return false;
            }

            // collision
            return true;
        }
	};
}