#include "Camera.h"

Camera::Camera(float aspectRatio) : mAspectRatio(aspectRatio) {
	BuildViewProjectionMatrix();
}

Camera::~Camera() {
}

void Camera::OnResize(UINT newClientWidth, UINT newClientHeight) {
	if (newClientHeight <= 0) { return; }

	mAspectRatio = (float)newClientWidth / (float)newClientHeight;
	BuildViewProjectionMatrix();
}

DirectX::XMFLOAT4X4 Camera::GetViewProjection() const {
	return mViewProjection;
}

void Camera::BuildViewProjectionMatrix() {
	// build projection
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		mFovY,
		mAspectRatio,
		mNearPlane,
		mFarPlane
	);
	XMStoreFloat4x4(&mProjection, P);

	// build view
	DirectX::XMVECTOR pos = XMLoadFloat4(&mPos);
	DirectX::XMVECTOR target = XMLoadFloat4(&mTarget);
	DirectX::XMVECTOR up = XMLoadFloat4(&mUp);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	// build view projection
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProjection);
	DirectX::XMMATRIX ViewProj = view * proj;
	XMStoreFloat4x4(&mViewProjection, ViewProj);
}