#pragma once

#include "../../../../../Math/EngineMath.h"

namespace Engine::EngineResources {

	struct MaterialData {
		Vector4 DiffuseAlbedo;
		Vector3 FresnelR0;
		float Roughness = 0.25f;
		Matrix4x4 MatTransform;

		// remember when you update this struct
		// also update 
		// 1. DX12ResourceDataStructures
		// 2. cbuffer in the shaders
	};
}