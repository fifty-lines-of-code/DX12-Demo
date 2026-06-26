#pragma once

#include "../../../../Engine/Math/EngineMath.h"
#include "../Lights/LightData.h"

namespace Engine::EngineWorld {

	// TODO:
	// find a better way to do the % 256
	// not to mention we are hardcoding it
	// 256 is a DirectX12 design
	// may not be the same for Vulkan, Playstation, etc

	struct EntityConstantBufferData {
		Matrix4x4 World;
		uint32_t SubMeshPad[48];

		// remember when you update this struct
		// also update 
		// 1. DX12FrameResource
		// 2. cbuffer in the shaders
	};
	static_assert((sizeof(EntityConstantBufferData) % 256) == 0,
		"Critical: Constant Buffer struct size must be a multiple of 256 bytes.");

	struct EntitySubMeshConstantBufferData {
		uint32_t MaterialID = 0;
		uint32_t TextureID = 0;
		uint32_t SubMeshPad[62];

		// remember when you udate this struct
		// also update
		// 1. DX12FrameResource
		// 2. cbuffer in shaders

		// we've made this 256 bytes as we are updating the
		// entire submesh array for each entity in one go
		// thus we need this on the cpu size to be
		// multiple of 256 so that the data on cpu side
		// is aligned correctly when we transfer it over
	};
	static_assert((sizeof(EntitySubMeshConstantBufferData) % 256) == 0,
		"Critical: Constant Buffer struct size must be a multiple of 256 bytes.");

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

		uint32_t SubMeshPad[40];
	};

	static_assert((sizeof(PerPassConstantBufferData) % 256) == 0,
		"Critical: Constant Buffer struct size must be a multiple of 256 bytes.");
}