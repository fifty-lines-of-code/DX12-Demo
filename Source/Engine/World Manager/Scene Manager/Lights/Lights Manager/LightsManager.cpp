#include "LightsManager.h"

namespace Engine {

	LightsManager::LightsManager() :
		mAmbientLight(0.25f, 0.25f, 0.35f, 1.0f)
	{}

	LightsManager::~LightsManager() {}

	bool LightsManager::Initialize() { 
		// todo:
		return true;
	}

	void LightsManager::Load(Vector3 sunStrength, Vector3 sunDirection) {
		// todo

		mGlobalSun.SetStrength(sunStrength);
		// lets ensure we normalize the direction here
		sunDirection.Normalize();
		mGlobalSun.SetDirection(sunDirection);
	}

	void LightsManager::Update(float deltaTime) {
		// todo:
		mGlobalSun.Update(deltaTime);
	}

	const Vector4& LightsManager::GetAmbientLight() const noexcept { return mAmbientLight; }

	const LightData& LightsManager::GetLightsData() const noexcept {
		return mGlobalSun.GetLightData();
	}
}