#include "Camera.h"

#include <cmath>

Camera::Camera(float aspectRatio) :
	mAspectRatio(aspectRatio) {
	BuildProjectionMatrix();
}

Camera::~Camera() {}

void Camera::Initialize(DirectX::XMFLOAT4 playerPosition) {
	mTarget = playerPosition;

	UpdateCenter(0, 0, 0);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

void Camera::Update(const DirectX::XMFLOAT4& playerPosition, float deltaTime, float rightJoystickX, float rightJoystickY) {
	mTarget = playerPosition;

	UpdateCenter(deltaTime, rightJoystickX, rightJoystickY);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

const DirectX::XMFLOAT4X4* Camera::GetViewProjection() const {
	return &mViewProjection;
}

void Camera::OnResize(UINT newClientWidth, UINT newClientHeight) {
	if (newClientHeight <= 0) { return; }

	mAspectRatio = (float)newClientWidth / (float)newClientHeight;
	BuildViewProjectionMatrix();
}

void Camera::UpdateCenter(float deltaTime, float rightJoystickX, float rightJoystickY) {
	// calculate target yaw and pitch
	mYaw = mYaw - (rightJoystickX * mYawSpeed * deltaTime);
	mPitch = mPitch - (rightJoystickY * mPitchSpeed * deltaTime);
	mPitch = std::fmin(mPitch, mPitchMax);
	mPitch = std::fmax(mPitch, mPitchMin);

	DirectX::XMFLOAT4 offset = DirectX::XMFLOAT4(0.f, 0.f, 0.f,1.f);

	offset.y = mRadius * std::sin(mPitch);
	// remember offset in -z cause we want the camera behind the player
	offset.z = -(mRadius * std::cos(mPitch) * std::cos(mYaw));
	offset.x = mRadius * std::cos(mPitch) * std::sin(mYaw);
	offset.w = 1.f; // to be safe even though we initialized offset with 1 for w

	DirectX::XMVECTOR targetVector = DirectX::XMLoadFloat4(&mTarget);
	DirectX::XMVECTOR offsetVector = DirectX::XMLoadFloat4(&offset);

	DirectX::XMStoreFloat4(
		&mCenter,
		DirectX::XMVectorAdd(targetVector, offsetVector)
	);
}

void Camera::BuildViewMatrix() {
	DirectX::XMVECTOR pos = XMLoadFloat4(&mCenter);
	DirectX::XMVECTOR target = XMLoadFloat4(&mTarget);
	DirectX::XMVECTOR up = XMLoadFloat4(&mUp);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void Camera::BuildProjectionMatrix() {
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		mFovY,
		mAspectRatio,
		mNearPlane,
		mFarPlane
	);
	XMStoreFloat4x4(&mProjection, P);
}

void Camera::BuildViewProjectionMatrix() {
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProjection);
	DirectX::XMMATRIX view = XMLoadFloat4x4(&mView);
	DirectX::XMMATRIX ViewProj = view * proj;
	XMStoreFloat4x4(&mViewProjection, ViewProj);
}