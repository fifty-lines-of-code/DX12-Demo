#pragma once

#include "../../Helper/MathHelper.h"
#include <wtypes.h>

class Camera {
public:
	Camera(float aspectRatio);
	~Camera();

	void Initialize(DirectX::XMFLOAT4 playerPosition);
	void Update(const DirectX::XMFLOAT4& playerPosition, float deltaTime, float rightJoystickX, float rightJoystickY);
	const DirectX::XMFLOAT4X4* GetViewProjection() const;

	void OnResize(UINT newClientWidth, UINT newClientHeight);

private:
	float mAspectRatio;
	float mFovY = 0.25 * MathHelper::Pi;
	float mNearPlane = 1.0;
	float mFarPlane = 1000.f;

	float mYaw = 0.01;
	float mPitch = DirectX::XMConvertToRadians(30);
	float mPitchMin = DirectX::XMConvertToRadians(-1);
	float mPitchMax = DirectX::XMConvertToRadians(45);
	float mRadius = 4.f;
	float mYawSpeed = .25f;
	float mPitchSpeed = .25f;

	DirectX::XMFLOAT4 mCenter = DirectX::XMFLOAT4(0, 2, -5.0, 1.f);
	DirectX::XMFLOAT4 mTarget = DirectX::XMFLOAT4(0, 0, 0, 1.f);
	DirectX::XMFLOAT4 mUp = DirectX::XMFLOAT4(0, 1, 0, 1.f);

	DirectX::XMFLOAT4X4 mProjection = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mViewProjection = MathHelper::Identity4x4();
	const float mTrackingSpeed = 4.f;

private:
	void UpdateCenter(float deltaTime, float rightJoystickX, float rightJoystickY);
	void BuildViewMatrix();
	void BuildProjectionMatrix();
	void BuildViewProjectionMatrix();
};