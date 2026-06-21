#pragma once

#include "../Math/EngineMath.h"
#include "../Math/MathHelper.h"
#include <wtypes.h>

namespace Engine {

	class Camera {
	public:
		Camera();
		~Camera();

		void Initialize(const Vector3& target);

		void UpdateWithInputSystem(
			float deltaTime,
			float rightJoystickX,
			float rightJoystickY
		);
		void UpdateWithTarget(const Vector3& target);

		const Matrix4x4& GetViewProjection() const;
		const BasisVectors& GetBasisVectors() const;
		const Vector3& GetPosition() const noexcept;

		void OnResize(UINT newClientWidth, UINT newClientHeight);

	private:
		Matrix4x4 mView;
		Matrix4x4 mProjection;
		Matrix4x4 mViewProjection;
		BasisVectors mBasisVectors;

		Vector3 mCenter = Vector3(0, 0, 0);
		Vector3 mTarget = Vector3(0, 0, 0);
		Vector3 mWorldUp = Vector3(0, 1, 0);
		Vector3 mOffset = Vector3(0.f, 0.f, 0.f);

		float mAspectRatio;
		float mFovY = MathHelper::Pi_Divide_By_4;
		float mNearPlane = 1.0f;
		float mFarPlane = 1000.f;

		float mYaw = 0.01f;
		float mPitch = MathHelper::ConvertToRadians(30);
		float mPitchMin = MathHelper::ConvertToRadians(-1);
		float mPitchMax = MathHelper::ConvertToRadians(45);
		float mRadius = 5.75f;
		float mYawSpeed = .75f;
		float mPitchSpeed = .5f;
		const float mTrackingSpeed = 4.f;

	private:
		void UpdateTarget(const Vector3& target);
		void UpdateYawPitchAndOffset(float deltaTime, float rightJoystickX, float rightJoystickY);
		void BuildViewMatrix();
		void BuildProjectionMatrix();
		void BuildViewProjectionMatrix();
	};
}