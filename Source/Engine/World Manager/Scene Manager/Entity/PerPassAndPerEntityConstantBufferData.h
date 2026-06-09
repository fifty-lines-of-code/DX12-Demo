#pragma once

#include "../../../../Engine/Math/EngineMath.h"
#include "../Lights/LightData.h"

namespace Engine {

	struct EntityConstantBufferData {
		Engine::Matrix4x4 World;
		uint32_t MaterialID;
	};

	using LightsArray16 = LightData[16];

	struct PerPassConstantBufferData {
		Engine::Matrix4x4 ViewProjectionTranspose;
		Engine::Vector4 AmbientLight;
		Engine::Vector3 EyePosW;
		float PassPad0;
		LightsArray16 Lights;
	};
}