#pragma once

#include <cstdint>

#include "LightData.h"

namespace Engine {

	enum class LightType : uint32_t {
		Directional,
		Point,
		Spot
	};

	class ILight {
	public:
		ILight(LightType type);
		virtual ~ILight();

		virtual void Update(float deltaTime) = 0;

		const LightData& GetLightData() const noexcept;
		LightType GetLightType() const noexcept;

	protected:
		LightData mData;
		LightType mType;
	};
}