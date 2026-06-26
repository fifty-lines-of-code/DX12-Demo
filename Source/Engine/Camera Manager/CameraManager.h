#pragma once

#include "Main Camera/MainCamera.h"
#include "../Simulation/SimulationDataStructures.h"

namespace Engine::EngineCamera {

	enum class CameraType : uint32_t {
		MAIN,
		REFLECTED,
		COUNT,
		INVALID
	};

	class CameraManager {
	public:
		CameraManager() = default;
		~CameraManager() = default;

		CameraManager(const CameraManager& rhs) = delete;
		CameraManager& operator=(const CameraManager& rhs) = delete;
		CameraManager(CameraManager&&) = delete;
		CameraManager& operator=(CameraManager&&) = delete;

		bool Initialize(const Vector3& mainCameraTarget);

		void UpdateMainCameraWithInputSystem(
			float deltaTime,
			float rightJoystickX,
			float rightJoystickY
		);
		void UpdateMainCameraWithTarget(const Vector3& target);

		void UpdateReflectedCameraWithSimulationData(
			const EngineSimulation::MirrorPlaneQueryResult& result
		);

		const Matrix4x4& GetMainCameraViewProjection() const;
		const BasisVectors& GetMainCameraBasisVectors() const;
		const Vector3& GetMainCameraPosition() const noexcept;

		void OnResize(
			UINT newClientWidth,
			UINT newClientHeight
		);

	private:
		MainCamera mMainCamera;
	};
}