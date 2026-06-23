#pragma once

#include "../../../../Engine/Math/EngineMath.h"
#include "../Lights/LightData.h"

namespace Engine::EngineWorld {

	struct EntityConstantBufferData {
		Matrix4x4 World;

		// remember when you update this struct
		// also update 
		// 1. DX12FrameResource
		// 2. cbuffer in the shaders
	};

	struct EntitySubMeshConstantBufferData {
		uint32_t MaterialID = 0;
		uint32_t TextureID = 0;
		Vector2 SubMeshPad0;

		// remember when you udate this struct
		// also update
		// 1. DX12FrameResource
		// 2. cbuffer in shaders
	};

	using LightsArray16 = LightData[16];

	struct PerPassConstantBufferData {
		Matrix4x4 ViewProjectionTranspose;
		Vector4 AmbientLight;
		Vector3 EyePosW;
		float PassPad0;
		LightsArray16 Lights;

		// remember when you update this struct
		// also update 
		// 1. DX12FrameResource
		// 2. cbuffer in the shaders
	};
}