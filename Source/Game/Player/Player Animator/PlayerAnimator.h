#pragma once

#include <DirectXMath.h>

struct CameraForwardAndRightVectors;

class PlayerAnimator {
public:
	PlayerAnimator();
	~PlayerAnimator();

	bool MoveAndRotatePlayer(
		float deltaTime,
		DirectX::XMFLOAT3 movement,
		DirectX::XMFLOAT3* center, 
		float* currentRotation,
		float walkingRunningSpeed,
		float rotationSpeed, 
		float movementLengthSquared
	);
	bool PerformBackwardsDash(
		float deltaTime, 
		DirectX::XMFLOAT3* center, 
		DirectX::XMFLOAT3* const forward,
		float backwardsDashDistance,
		float animationDuration
	);
	bool IsBackwardsDashAnimationComplete() const;

private:
	// dash animation data
	DirectX::XMFLOAT3 mDashStartPosition = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 mDashTargetPosition = DirectX::XMFLOAT3(0, 0, 0);
	float mDashAnimationTimer = 0.5f;
	float mIsPerformingBackwardsDash = false;
	float mIsBackwardsDashAnimationComplete = true;
	DirectX::XMFLOAT3 mDashDirection;
	

private:
	DirectX::XMFLOAT3 CalculateMovementVectorFrom(CameraForwardAndRightVectors forwardAndRightVectors, float leftStickX, float leftStickY);
	void RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movement, float rotationSpeed, float* currentRotation);
};