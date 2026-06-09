#pragma once

#include "../ILight.h"

namespace Engine {

	class DirectionalLight : public ILight {
	public:
		DirectionalLight();
		~DirectionalLight();

		void Update(float deltaTime) override;
		void SetStrength(Vector3 strength);
		void SetDirection(Vector3 direction);

	private:

	};
}