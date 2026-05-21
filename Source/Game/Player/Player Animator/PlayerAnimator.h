#pragma once

#include <DirectXMath.h>

struct CameraForwardAndRightVectors;

class PlayerAnimator {
public:
	PlayerAnimator(float currentRotation);
	~PlayerAnimator();

	bool MoveAndRotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, DirectX::XMFLOAT3* center, float walkingRunningSpeed, float rotationSpeed);
	bool PerformBackwardsDash(float deltaTime, DirectX::XMFLOAT3* center, float backwardsDashDistance, float animationDuration);
	bool IsBackwardsDashAnimationComplete() const;

private:
	// dash animation data
	DirectX::XMFLOAT3 mDashStartPosition = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 mDashTargetPosition = DirectX::XMFLOAT3(0, 0, 0);
	float mDashAnimationTimer = 0.f;
	float mIsPerformingBackwardsDash = false;
	float mBackwardsDashAnimationHasCompleted = true;
	float mCurrentRotation;

private:
	DirectX::XMFLOAT3 CalculateMovementVectorFrom(CameraForwardAndRightVectors forwardAndRightVectors, float leftStickX, float leftStickY);
	void RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, float rotationSpeed);
};