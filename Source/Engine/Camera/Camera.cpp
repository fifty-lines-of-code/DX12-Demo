#include "Camera.h"

#include <cmath>
#include <DirectXMath.h>

Camera::Camera(float aspectRatio) :
	mAspectRatio(aspectRatio) {
	BuildProjectionMatrix();
}

Camera::~Camera() {}

void Camera::Initialize(const Engine::Vector3* target) {
	UpdateTarget(target);

	UpdateYawPitchAndOffset(0, 0, 0);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

void Camera::UpdateWithInputSystem(float deltaTime, float rightJoystickX, float rightJoystickY) {
	UpdateYawPitchAndOffset(deltaTime, rightJoystickX, rightJoystickY);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

void Camera::UpdateWithTarget(const Engine::Vector3* target) {
	UpdateTarget(target);

	DirectX::XMVECTOR targetVector = DirectX::XMLoadFloat3(&mTarget.AsXMFLOAT3());
	DirectX::XMVECTOR offsetVector = DirectX::XMLoadFloat3(&mOffset.AsXMFLOAT3());

	DirectX::XMStoreFloat3(
		&mCenter.AsXMFLOAT3(),
		DirectX::XMVectorAdd(targetVector, offsetVector)
	);

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

const Engine::Matrix4x4* Camera::GetViewProjection() const {
	return &mViewProjection;
}

const Engine::BasisVectors* Camera::GetBasisVectors() const {
	return &basisVectors;
}

void Camera::OnResize(UINT newClientWidth, UINT newClientHeight) {
	if (newClientHeight <= 0) { return; }

	mAspectRatio = (float)newClientWidth / (float)newClientHeight;
	BuildProjectionMatrix();
	BuildViewProjectionMatrix();
}

void Camera::UpdateTarget(const Engine::Vector3* target) {
	mTarget.x = target->x;
	mTarget.y = target->y;
	mTarget.z = target->z;
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

	// Update forward and right vectors

	// In a traditional coordinate system, to find a vector 90 degrees clockwise ("Right") from a Forward vector F,
	// We use the standard 2D perpendicular rule:
	// swap the components and negate the new Z component
	
	// since DX12 is a left hand coordinate system and
	// Math uses a right hand coordinate system
	// we have to use (-mYaw) to convert between the two systems
	// thus sin(-mYaw) = -sin(mYaw) and cos(-mYaw) = cos(mYaw)
	// and forward x and z come from simple trignometry
	// if we draw x, z, and the pitch out on paper

	basisVectors.forward = Engine::Vector3(-std::sin(mYaw), 0.f, std::cos(mYaw));
	basisVectors.right = Engine::Vector3(std::cos(mYaw), 0.f, -(-std::sin(mYaw)));
}

void Camera::BuildViewMatrix() {
	DirectX::XMVECTOR pos = XMLoadFloat3(&mCenter.AsXMFLOAT3());
	DirectX::XMVECTOR target = XMLoadFloat3(&mTarget.AsXMFLOAT3());
	DirectX::XMVECTOR up = XMLoadFloat3(&mWorldUp.AsXMFLOAT3());

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(
		&mView.AsXMFLOAT4X4(),
		view
	);
}

void Camera::BuildProjectionMatrix() {
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		mFovY,
		mAspectRatio,
		mNearPlane,
		mFarPlane
	);
	XMStoreFloat4x4(
		&mProjection.AsXMFLOAT4X4(),
		P
	);
}

void Camera::BuildViewProjectionMatrix() {
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProjection.AsXMFLOAT4X4());
	DirectX::XMMATRIX view = XMLoadFloat4x4(&mView.AsXMFLOAT4X4());
	DirectX::XMMATRIX ViewProj = view * proj;
	XMStoreFloat4x4(
		&mViewProjection.AsXMFLOAT4X4(),
		ViewProj
	);
}