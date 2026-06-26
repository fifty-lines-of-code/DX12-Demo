#pragma once

#include <array>
#include "../../../EngineConfig.h"
#include "../../../Math/EngineMath.h"

namespace Engine::EngineWorld {

	struct EntitySubMeshMaterialData {
		uint32_t MaterialID = 0;
		uint32_t TextureID = 0;
	};

	struct EntityRenderData {

        Matrix4x4 WorldMatrix;
        Vector3 SurfaceNormal;

        std::array<EntitySubMeshMaterialData, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> SubMeshMaterials;

		void RebuildWorldMatrix(
			const Vector3& center,
			const Vector3& scale,
			const BasisVectors& basisVectors
		) {
			RebuildWorldMatrix_Internal(
				center,
				scale,
				basisVectors
			);
		}

	private:
		void RebuildWorldMatrix_Internal(
			const Vector3& entityCenter,
			const Vector3& entityScale,
			const BasisVectors& entityBasisVectors
		) {
			// EXPLICIT SRT COMPOSITION (Intentionally expanded for clarity)
			// Demonstrates row-major DirectXMath convention:
			//   Row 0 = Right vector, Row 1 = Up vector, Row 2 = Forward vector
			//   Row 3 = Translation
			// Composition order: World = Scale * Rotation * Translation
			
			// todo in Production: Use DX methods to build SRT and then W
			// DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(mScaleX, mScaleY, mScaleZ);

			// Matrix4x4 default-constructs to identity. 
			// Only non-identity elements are overwritten below.
			Engine::Matrix4x4 scale;
			Engine::Matrix4x4 rotation;
			Engine::Matrix4x4 translation;

			// Set Scale
			scale.m[0][0] = entityScale.x;
			scale.m[1][1] = entityScale.y;
			scale.m[2][2] = entityScale.z;

			// Set Rotation
			// Row 0: Right
			rotation.m[0][0] = entityBasisVectors.Right.x;
			rotation.m[0][1] = entityBasisVectors.Right.y;
			rotation.m[0][2] = entityBasisVectors.Right.z;
			rotation.m[0][3] = 0.f;

			// Row 1: Up
			rotation.m[1][0] = entityBasisVectors.Up.x;
			rotation.m[1][1] = entityBasisVectors.Up.y;
			rotation.m[1][2] = entityBasisVectors.Up.z;
			rotation.m[1][3] = 0.f;

			// Row 2: Forward
			rotation.m[2][0] = entityBasisVectors.Forward.x;
			rotation.m[2][1] = entityBasisVectors.Forward.y;
			rotation.m[2][2] = entityBasisVectors.Forward.z;
			rotation.m[2][3] = 0.f;

			// Set Translation
			translation.m[3][0] = entityCenter.x;
			translation.m[3][1] = entityCenter.y;
			translation.m[3][2] = entityCenter.z;
			translation.m[3][3] = 1.0f;

			// lets read and write to our Matrix4x4 as an XMFLOAT4x4 so that
			// we get access to fast SIMD math operations
			// from my understanding there should be 0 performance penalty for this 
			// cast and it allows us to keep our code clean,
			// without having to use too many XMMatrix* methods

			DirectX::XMMATRIX scaleRotation = DirectX::XMMatrixMultiply(
				DirectX::XMLoadFloat4x4(&scale.AsXMFLOAT4X4()),
				DirectX::XMLoadFloat4x4(&rotation.AsXMFLOAT4X4())
			);

			DirectX::XMMATRIX worldXM = DirectX::XMMatrixMultiply(
				scaleRotation,
				DirectX::XMLoadFloat4x4(&translation.AsXMFLOAT4X4())
			);

			DirectX::XMStoreFloat4x4(
				&WorldMatrix.AsXMFLOAT4X4(),
				worldXM
			);
		}
	};
}