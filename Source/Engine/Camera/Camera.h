#pragma once

#include "../MathHelper.h"
#include <wtypes.h>

class Camera {
public:
	Camera(float aspectRatio);
	~Camera();

	void OnResize(UINT newClientWidth, UINT newClientHeight);
	DirectX::XMFLOAT4X4 GetViewProjection() const;

private:
	float mAspectRatio;
	float mFovY = 0.25 * MathHelper::Pi;
	float mNearPlane = 1.0;
	float mFarPlane = 1000.f;
	DirectX::XMFLOAT4 mPos = DirectX::XMFLOAT4(0, 2, -5.0, 1.f);
	DirectX::XMFLOAT4 mTarget = DirectX::XMFLOAT4(0, 0, 0, 1.f);
	DirectX::XMFLOAT4 mUp = DirectX::XMFLOAT4(0, 1, 0, 1.f);
	DirectX::XMFLOAT4X4 mProjection = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mViewProjection = MathHelper::Identity4x4();

private:
	void BuildViewProjectionMatrix();
};