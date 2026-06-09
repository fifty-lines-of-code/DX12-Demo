#include "ILight.h"

namespace Engine {
	ILight::ILight(LightType type) : 
		mType(type) 
	{}

	ILight::~ILight() {}

	const LightData& ILight::GetLightData() const noexcept { return mData; }

	LightType ILight::GetLightType() const noexcept { return mType; }

}