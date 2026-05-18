#include "Camera.h"

#include <cmath>

Camera::Camera(float aspectRatio) :
	mAspectRatio(aspectRatio) {
	BuildProjectionMatrix();
}

Camera::~Camera() {}

void Camera::Initialize(DirectX::XMFLOAT4 playerPosition) {
	mTarget = playerPosition;

	UpdateYawPitchAndOffset(0, 0, 0);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

void Camera::UpdateWithInputSystem(float deltaTime, float rightJoystickX, float rightJoystickY) {
	UpdateYawPitchAndOffset(deltaTime, rightJoystickX, rightJoystickY);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

void Camera::UpdateWithTarget(DirectX::XMFLOAT4 target) {
	mTarget = target;

	DirectX::XMVECTOR targetVector = DirectX::XMLoadFloat4(&mTarget);
	DirectX::XMVECTOR offsetVector = DirectX::XMLoadFloat4(&mOffset);

	DirectX::XMStoreFloat4(
		&mCenter,
		DirectX::XMVectorAdd(targetVector, offsetVector)
	);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

const DirectX::XMFLOAT4X4* Camera::GetViewProjection() const {
	return &mViewProjection;
}

CameraForwardAndRightVectors Camera::GetForwardAndRightVectors() const {
	return mForwardAndRight;
}

void Camera::OnResize(UINT newClientWidth, UINT newClientHeight) {
	if (newClientHeight <= 0) { return; }

	mAspectRatio = (float)newClientWidth / (float)newClientHeight;
	BuildProjectionMatrix();
	BuildViewProjectionMatrix();
}

void Camera::UpdateYawPitchAndOffset(float deltaTime, float rightJoystickX, float rightJoystickY) {
	// calculate target yaw and pitch
	mYaw = mYaw - (rightJoystickX * mYawSpeed * deltaTime);
	mPitch = mPitch - (rightJoystickY * mPitchSpeed * deltaTime);
	
	// clamp pitch to prevent gimbal lock
	mPitch = std::fmin(mPitch, mPitchMax);
	mPitch = std::fmax(mPitch, mPitchMin);

	mOffset.y = mRadius * std::sin(mPitch);
	// remember offset in -z cause we want the camera behind the player
	mOffset.z = -(mRadius * std::cos(mPitch) * std::cos(mYaw));
	mOffset.x = mRadius * std::cos(mPitch) * std::sin(mYaw);
	// to be safe even though we initialized offset with 1 for w
	mOffset.w = 1.f;

	// Update forward and right vectors

	// In a traditional coordinate system, to find a vector 90 degrees clockwise ("Right") from a Forward vector F,
	// We use the standard 2D perpendicular rule:
	// swap the components and negate the new Z component
	
	// since DX12 is a left hand coordinate system and
	// Math uses a right hand coordinate system
	// we have to use (-mYaw) to convert between the two systems
	// thus sin(-mYaw) = -sin(mYaw) and cos(-mYaw) = cos(mYaw)
	// and forward x and z come from simple tignometry
	// if we draw x, z, and the pitch out on paper

	mForwardAndRight.forward = DirectX::XMFLOAT3(-std::sin(mYaw), 0.f, std::cos(mYaw));
	mForwardAndRight.right = DirectX::XMFLOAT3(std::cos(mYaw), 0.f, -(-std::sin(mYaw)));
}

void Camera::BuildViewMatrix() {
	DirectX::XMVECTOR pos = XMLoadFloat4(&mCenter);
	DirectX::XMVECTOR target = XMLoadFloat4(&mTarget);
	DirectX::XMVECTOR up = XMLoadFloat4(&mWorldUp);

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