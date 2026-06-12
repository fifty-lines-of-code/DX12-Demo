#pragma once

#include "../../../../Engine/Math/EngineMath.h"
#include "../Lights/LightData.h"

namespace Engine {

	struct EntityConstantBufferData {
		Matrix4x4 World;
		uint32_t MaterialID;
		uint32_t TextureID;

		// remember when you update this struct
		// also update 
		// 1. DX12FrameResource
		// 2. cbuffer in the shaders
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