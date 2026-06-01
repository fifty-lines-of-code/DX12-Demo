#pragma once

#include "../Math/EngineMath.h"
#include "../Math/MathHelper.h"
#include <wtypes.h>

class Camera {
public:
	Camera();
	~Camera();

	void Initialize(const Engine::Vector3& target);
	
	void UpdateWithInputSystem(float deltaTime, float rightJoystickX, float rightJoystickY);
	void UpdateWithTarget(const Engine::Vector3& target);

	const Engine::Matrix4x4* GetViewProjection() const;
	const Engine::BasisVectors& GetBasisVectors() const;

	void OnResize(UINT newClientWidth, UINT newClientHeight);

private:
	float mAspectRatio;
	float mFovY = Engine::MathHelper::Pi_Divide_By_4;
	float mNearPlane = 1.0;
	float mFarPlane = 1000.f;

	float mYaw = 0.01;
	float mPitch = Engine::MathHelper::ConvertToRadians(30);
	float mPitchMin = Engine::MathHelper::ConvertToRadians(-1);
	float mPitchMax = Engine::MathHelper::ConvertToRadians(45);
	float mRadius = 4.f;
	float mYawSpeed = .75f;
	float mPitchSpeed = .5f;

	Engine::Vector3 mCenter = Engine::Vector3(0, 0, 0);
	Engine::Vector3 mTarget =Engine::Vector3(0, 0, 0);
	Engine::Vector3 mWorldUp = Engine::Vector3(0, 1, 0);
	Engine::Vector3 mOffset = Engine::Vector3(0.f, 0.f, 0.f);

	Engine::Matrix4x4 mView;
	Engine::Matrix4x4 mProjection;
	Engine::Matrix4x4 mViewProjection;
	const float mTrackingSpeed = 4.f;
	Engine::BasisVectors mBasisVectors;

private:
	void UpdateTarget(const Engine::Vector3& target);
	void UpdateYawPitchAndOffset(float deltaTime, float rightJoystickX, float rightJoystickY);
	void BuildViewMatrix();
	void BuildProjectionMatrix();
	void BuildViewProjectionMatrix();
};