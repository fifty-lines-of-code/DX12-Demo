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
	) noexcept {
		const BasisVectors& mainCameraBasisvectors = mMainCamera.GetBasisVectors();

		mReflectedCamera.UpdateWithMainCameraData(
			mMainCamera.GetCenter(),
			mainCameraBasisvectors.Forward,
			mainCameraBasisvectors.Up,
			mMainCamera.GetFovY(),
			mMainCamera.GetAspectRatio(),
			mMainCamera.GetNearPlane(),
			mMainCamera.GetFarPlane(),
			result
		);
	}

	const Matrix4x4& CameraManager::GetViewProjection(CameraType type) const noexcept {
		if (type == CameraType::REFLECTED) {
			return mReflectedCamera.GetViewProjection();
		}

		return mMainCamera.GetViewProjection();
	}

	const BasisVectors& CameraManager::GetMainCameraBasisVectors() const {
		return mMainCamera.GetBasisVectors();
	}

	const Vector3& CameraManager::GetCameraCenter(CameraType type) const noexcept {
		if (type == CameraType::REFLECTED) {
			return mReflectedCamera.GetCenter();
		}
		return mMainCamera.GetCenter();
	}

	void CameraManager::OnResize(UINT newClientWidth, UINT newClientHeight) {
		mMainCamera.OnResize(newClientWidth, newClientHeight);
	}
}