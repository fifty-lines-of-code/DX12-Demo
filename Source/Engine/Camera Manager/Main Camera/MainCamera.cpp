#include "MainCamera.h"

#include "../CameraHelper.h"
#include <cmath>
#include <DirectXMath.h>

namespace Engine::EngineCamera {

	bool MainCamera::Initialize(const Engine::Vector3& target) {
		UpdateTarget(target);

		UpdateYawPitchAndOffset(0, 0, 0);

		BuildViewMatrix();

		return true;
	}

	void MainCamera::UpdateWithInputSystem(float deltaTime, float rightJoystickX, float rightJoystickY) {
		UpdateYawPitchAndOffset(
			deltaTime, 
			rightJoystickX, 
			rightJoystickY
		);

		BuildViewMatrix();
		BuildViewProjectionMatrix();
	}

	void MainCamera::UpdateWithTarget(const Engine::Vector3& target) {
		UpdateTarget(target);

		DirectX::XMVECTOR targetVector = DirectX::XMLoadFloat3(
			&mMainCameraData.Target.AsXMFLOAT3()
		);
		DirectX::XMVECTOR offsetVector = DirectX::XMLoadFloat3(
			&mMainCameraData.Offset.AsXMFLOAT3()
		);

		DirectX::XMStoreFloat3(
			&mMainCameraData.Center.AsXMFLOAT3(),
			DirectX::XMVectorAdd(targetVector, offsetVector)
		);

		BuildViewMatrix();
		BuildViewProjectionMatrix();
	}

	const Matrix4x4& MainCamera::GetViewProjection() const {
		return mMainCameraData.ViewProjection;
	}

	const BasisVectors& MainCamera::GetBasisVectors() const {
		return mMainCameraData.BasisVectors;
	}

	const Vector3& MainCamera::GetCenter() const noexcept { 
		return mMainCameraData.Center;
	}

	float MainCamera::GetFovY() const noexcept {
		return mMainCameraData.FovY;
	}

	float MainCamera::GetAspectRatio() const noexcept {
		return mMainCameraData.AspectRatio;
	}

	float MainCamera::GetNearPlane() const noexcept {
		return mMainCameraData.NearPlane;
	}

	float MainCamera::GetFarPlane() const noexcept {
		return mMainCameraData.FarPlane;
	}

	void MainCamera::OnResize(UINT newClientWidth, UINT newClientHeight) {
		if (newClientHeight <= 0) { return; }

		mMainCameraData.AspectRatio = (float)newClientWidth / (float)newClientHeight;
		BuildProjectionMatrix();
		BuildViewProjectionMatrix();
	}

#pragma region Private 

	void MainCamera::UpdateTarget(const Engine::Vector3& target) {
		mMainCameraData.Target = target;
	}

	void MainCamera::UpdateYawPitchAndOffset(
		float deltaTime, 
		float rightJoystickX, 
		float rightJoystickY
	) {
		// calculate target yaw and pitch
		mMainCameraData.Yaw -= (
			rightJoystickX * 
			mMainCameraData.YawSpeed * 
			deltaTime
		);

		mMainCameraData.Pitch -= (
			rightJoystickY * 
			mMainCameraData.PitchSpeed * 
			deltaTime
		);

		// clamp pitch to prevent gimbal lock
		mMainCameraData.Pitch = std::fmin(
			mMainCameraData.Pitch, 
			mMainCameraData.PitchMax
		);
		mMainCameraData.Pitch = std::fmax(
			mMainCameraData.Pitch,
			mMainCameraData.PitchMin
		);

		mMainCameraData.Offset.y = 
			mMainCameraData.Radius * 
			std::sin(mMainCameraData.Pitch);
		// remember offset in -z cause we want the camera behind the player
		mMainCameraData.Offset.z = -(
			mMainCameraData.Radius * 
			std::cos(mMainCameraData.Pitch) * 
			std::cos(mMainCameraData.Yaw)
		);

		mMainCameraData.Offset.x = 
			mMainCameraData.Radius *
			std::cos(mMainCameraData.Pitch) * 
			std::sin(mMainCameraData.Yaw);

		// Update forward and right vectors

		// In a traditional coordinate system, to find a vector 90 degrees clockwise ("Right") from a Forward vector F,
		// We use the standard 2D perpendicular rule:
		// swap the components and negate the new Z component

		// since DX12 is a left hand coordinate system and
		// Math uses a right hand coordinate system
		// we have to use (-Yaw) to convert between the two systems
		// thus sin(-mYaw) = -sin(Yaw) and cos(-Yaw) = cos(Yaw)
		// and forward x and z come from simple trignometry
		// if we draw x, z, and the pitch out on paper

		mMainCameraData.BasisVectors.Forward = Engine::Vector3(
			-std::sin(mMainCameraData.Yaw), 
			0.f, 
			std::cos(mMainCameraData.Yaw)
		);

		mMainCameraData.BasisVectors.Right = Engine::Vector3(
			std::cos(mMainCameraData.Yaw),
			0.f, 
			-(-std::sin(mMainCameraData.Yaw))
		);
	}

	void MainCamera::BuildViewMatrix() {
		DirectX::XMVECTOR pos = XMLoadFloat3(
			&mMainCameraData.Center.AsXMFLOAT3()
		);
		DirectX::XMVECTOR target = XMLoadFloat3(
			&mMainCameraData.Target.AsXMFLOAT3()
		);
		DirectX::XMVECTOR up = XMLoadFloat3(
			&mMainCameraData.WorldUp.AsXMFLOAT3()
		);

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(
			&mMainCameraData.View.AsXMFLOAT4X4(),
			view
		);
	}

	void MainCamera::BuildProjectionMatrix() {
		DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
			mMainCameraData.FovY,
			mMainCameraData.AspectRatio,
			mMainCameraData.NearPlane,
			mMainCameraData.FarPlane
		);
		XMStoreFloat4x4(
			&mMainCameraData.Projection.AsXMFLOAT4X4(),
			P
		);
	}

	void MainCamera::BuildViewProjectionMatrix() {
		DirectX::XMMATRIX proj = XMLoadFloat4x4(
			&mMainCameraData.Projection.AsXMFLOAT4X4()
		);
		DirectX::XMMATRIX view = XMLoadFloat4x4(
			&mMainCameraData.View.AsXMFLOAT4X4()
		);
		DirectX::XMMATRIX ViewProj = view * proj;
		XMStoreFloat4x4(
			&mMainCameraData.ViewProjection.AsXMFLOAT4X4(),
			ViewProj
		);

		CameraUtil::BuildViewProjection(
			mMainCameraData.View,
			mMainCameraData.Projection,
			mMainCameraData.ViewProjection
		);
	}

#pragma endregion
}