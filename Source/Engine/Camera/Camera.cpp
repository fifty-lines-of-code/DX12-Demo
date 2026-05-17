#include "Camera.h"

Camera::Camera(float aspectRatio) :
	mAspectRatio(aspectRatio),
	mTrackingOffset(DirectX::XMFLOAT3(0.0f, 2.5f, -4.0f)) {
	BuildProjectionMatrix();
}

Camera::~Camera() {}

void Camera::Initialize(DirectX::XMFLOAT4 playerPosition) {
	mTarget = playerPosition;

	mCenter.x = playerPosition.x + mTrackingOffset.x;
	mCenter.y = playerPosition.y + mTrackingOffset.y;
	mCenter.z = playerPosition.z + mTrackingOffset.z;
	mCenter.w = 1.f;

	BuildViewMatrix();
	BuildViewProjectionMatrix();
}

void Camera::Update(const DirectX::XMFLOAT4& playerPosition, float deltaTime) {
	mTarget = playerPosition;

	// Since we want to track player Fromsoft style, update mPosition
	// based on player position
	DirectX::XMVECTOR idealTarget = DirectX::XMVectorSet(
		playerPosition.x + mTrackingOffset.x,
		playerPosition.y + mTrackingOffset.y,
		playerPosition.z + mTrackingOffset.z,
		1.0f
	);

	DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat4(&mCenter);

	// Smoothly interpolate from current position towards target position
	// We clamp the blending factor to 1.0f max to prevent overshoot during severe lag spikes
	float blendFactor = mTrackingSpeed * deltaTime;
	if (blendFactor > 1.0f) { blendFactor = 1.0f; }

	DirectX::XMVECTOR newPos = DirectX::XMVectorLerp(currentPos, idealTarget, blendFactor);
	DirectX::XMStoreFloat4(&mCenter, newPos);

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