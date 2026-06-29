#pragma once

#include "../../../Math/EngineMath.h"

namespace Engine {

	// Be congnizant of the HLSL packing rules when defining structs

	struct LightData {
		// Light color
		Vector3 Strength;
		// point/spot lights only
		float FallOffStart;
		// directional/spot lights only
		Vector3 Direction;
		// point/spot lights only
		float FallOffEnd;
		// point/spot lights only
		Vector3 Position;
		// spot lights only
		float SpotPower;

		// remember when you update this struct
		// also update 
		// 1. DX12ResourceDataStructures
		// 2. cbuffer in the shaders
	};
}