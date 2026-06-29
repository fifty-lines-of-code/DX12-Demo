#pragma once

#include "../../Math/EngineMath.h"
#include "../../Math/MathHelper.h"
#include <wtypes.h>

namespace Engine::EngineCamera {

	struct MainCameraData {
		Matrix4x4 View;
		Matrix4x4 Projection;
		Matrix4x4 ViewProjection;
		BasisVectors BasisVectors;

		Vector3 Center = Vector3(0, 0, 0);
		Vector3 Target = Vector3(0, 0, 0);
		Vector3 WorldUp = Vector3(0, 1, 0);
		Vector3 Offset = Vector3(0.f, 0.f, 0.f);

		float AspectRatio = 0.f;;
		float FovY = MathHelper::Pi_Divide_By_4;
		float NearPlane = 1.0f;
		float FarPlane = 1000.f;

		float Yaw = 0.01f;
		float Pitch = MathHelper::ConvertToRadians(30);
		float PitchMin = MathHelper::ConvertToRadians(-1);
		float PitchMax = MathHelper::ConvertToRadians(45);
		float Radius = 5.75f;
		float YawSpeed = .75f;
		float PitchSpeed = .5f;
		const float TrackingSpeed = 4.f;
	};

	class MainCamera {
	public:
		MainCamera() = default;
		~MainCamera() = default;

		MainCamera(const MainCamera& rhs) = delete;
		MainCamera& operator=(const MainCamera& rhs) = delete;
		MainCamera(MainCamera&&) = delete;
		MainCamera& operator=(MainCamera&&) = delete;

		bool Initialize(const Vector3& target);

		void UpdateWithInputSystem(
			float deltaTime,
			float rightJoystickX,
			float rightJoystickY
		);
		void UpdateWithTarget(const Vector3& target);

		const Matrix4x4& GetViewProjection() const;
		const BasisVectors& GetBasisVectors() const;
		const Vector3& GetCenter() const noexcept;
		float GetFovY() const noexcept;
		float GetAspectRatio() const noexcept;
		float GetNearPlane() const noexcept;
		float GetFarPlane() const noexcept;

		void OnResize(
			UINT newClientWidth, 
			UINT newClientHeight
		);

	private:
		MainCameraData mMainCameraData;

	private:
		void UpdateTarget(const Vector3& target);
		void UpdateYawPitchAndOffset(float deltaTime, float rightJoystickX, float rightJoystickY);
		void BuildViewMatrix();
		void BuildProjectionMatrix();
		void BuildViewProjectionMatrix();
	};
}