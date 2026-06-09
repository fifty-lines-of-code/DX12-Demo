#pragma once

#include "../Directional Lights/DirectionalLight.h"

namespace Engine {

	class LightsManager {
	public:
		LightsManager();
		~LightsManager();

		bool Initialize(Vector3 sunStrength, Vector3 sunDirection);
		
		void Update(float deltaTime);

		const Vector4& GetAmbientLight() const noexcept;
		const LightData& GetLightsData() const noexcept;

	private:
		DirectionalLight mGlobalSun;
		Vector4 mAmbientLight;
	};
}