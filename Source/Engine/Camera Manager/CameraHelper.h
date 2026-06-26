#pragma once


#include <DirectXMath.h>
#include "../Math/EngineMath.h"

namespace Engine::EngineCamera {

	class CameraUtil {
	public:

		static void BuildViewProjection(
			const Matrix4x4& view,
			const Matrix4x4& projection,
			Matrix4x4& viewProjection
		) {
			DirectX::XMMATRIX xmView = XMLoadFloat4x4(
				&view.AsXMFLOAT4X4()
			);
			DirectX::XMMATRIX xmProj = XMLoadFloat4x4(
				&projection.AsXMFLOAT4X4()
			);

			DirectX::XMMATRIX ViewProj = xmView * xmProj;

			XMStoreFloat4x4(
				&viewProjection.AsXMFLOAT4X4(),
				ViewProj
			);
		}
	};
}