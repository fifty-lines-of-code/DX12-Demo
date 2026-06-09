#include "DirectionalLight.h"

namespace Engine {

	DirectionalLight::DirectionalLight() : 
		ILight(LightType::Directional)
	{}

	DirectionalLight::~DirectionalLight() {}

	void DirectionalLight::Update(float deltaTime) {

	}

	void DirectionalLight::SetStrength(Vector3 strength) { mData.Strength = strength; }

	void DirectionalLight::SetDirection(Vector3 direction) { mData.Direction = direction; }
}