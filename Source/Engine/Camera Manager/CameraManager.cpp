#include "CameraManager.h"

namespace Engine::EngineCamera {

	bool CameraManager::Initialize(const Vector3& mainCameraTarget) {
		return mMainCamera.Initialize(mainCameraTarget);
	}

	void CameraManager::UpdateMainCameraWithInputSystem(
		float deltaTime,
		float rightJoystickX,
		float rightJoystickY
	) {
		mMainCamera.UpdateWithInputSystem(
			deltaTime, 
			rightJoystickX,
			rightJoystickY
		);
	}

	void CameraManager::UpdateMainCameraWithTarget(const Vector3& target) {
		mMainCamera.UpdateWithTarget(target);
	}

	void CameraManager::UpdateReflectedCameraWithSimulationData(
		const EngineSimulation::MirrorPlaneQueryResult& result
	) {
		// todo:
	}

	const Matrix4x4& CameraManager::GetMainCameraViewProjection() const {
		return mMainCamera.GetViewProjection();
	}

	const BasisVectors& CameraManager::GetMainCameraBasisVectors() const {
		return mMainCamera.GetBasisVectors();
	}

	const Vector3& CameraManager::GetMainCameraPosition() const noexcept {
		return mMainCamera.GetPosition();
	}

	void CameraManager::OnResize(UINT newClientWidth, UINT newClientHeight) {
		mMainCamera.OnResize(newClientWidth, newClientHeight);
	}
}