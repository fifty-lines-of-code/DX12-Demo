#pragma once

#include "../Math/EngineMath.h"
#include "../Math/GeometryHelper.h"

namespace Engine::EnginePhysics {

	struct PhysicsBody {
		// current validated spatial state
		Matrix4x4 WorldMatrix;
		BasisVectors BasisVectors;
		Vector3 Center;
		Vector3 Scale;

		// transient simulation state
		Vector3 VelocityIntent;

		// bounds
		AABB LocalAABB;
		AABB WorldAABB;

		PhysicsBody(
			const Engine::Vector3& initCenter,
			const Engine::Vector3& initScale
		) : Center(initCenter),
			Scale(initScale), 
			BasisVectors()
		{}

		void UpdateProductionTransforms() {
			RebuildWorldMatrix(WorldMatrix, Center);
			GeometryHelper::CalculateAABB(
				LocalAABB,
				WorldMatrix, WorldAABB
			);
		}

	private:
		void RebuildWorldMatrix(
			Matrix4x4& world,
			Vector3& center
		) const {
			// todo: Use DX methods to build SRT and then W
			// DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(mScaleX, mScaleY, mScaleZ);
			Engine::Matrix4x4 scale;
			Engine::Matrix4x4 rotation;
			Engine::Matrix4x4 translation;

			// Set Scale
			scale.m[0][0] = Scale.x;
			scale.m[1][1] = Scale.y;
			scale.m[2][2] = Scale.z;

			// Set Rotation
			// Row 0: Right
			rotation.m[0][0] = BasisVectors.Right.x;
			rotation.m[0][1] = BasisVectors.Right.y;
			rotation.m[0][2] = BasisVectors.Right.z;
			rotation.m[0][3] = 0.f;

			// Row 1: Up
			rotation.m[1][0] = BasisVectors.Up.x;
			rotation.m[1][1] = BasisVectors.Up.y;
			rotation.m[1][2] = BasisVectors.Up.z;
			rotation.m[1][3] = 0.f;

			// Row 2: Forward
			rotation.m[2][0] = BasisVectors.Forward.x;
			rotation.m[2][1] = BasisVectors.Forward.y;
			rotation.m[2][2] = BasisVectors.Forward.z;
			rotation.m[2][3] = 0.f;

			// Set Translation
			translation.m[3][0] = center.x;
			translation.m[3][1] = center.y;
			translation.m[3][2] = center.z;
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
				&world.AsXMFLOAT4X4(),
				worldXM
			);
		}
	};
}