#pragma once

#include <DirectXMath.h>

struct CameraBasisVectors;

class PlayerAnimator {
public:
	PlayerAnimator();
	~PlayerAnimator();

	bool MoveAndRotatePlayer(
		float deltaTime,
		const DirectX::XMFLOAT3* const movement,
		DirectX::XMFLOAT3* const center, 
		float* currentRotation,
		float walkingRunningSpeed,
		float rotationSpeed
	);
	void RotatePlayer(
		float deltaTime,
		const DirectX::XMFLOAT3* const movement,
		float rotationSpeed,
		float* currentRotation
	);
	bool PerformBackwardsDash(
		float deltaTime, 
		DirectX::XMFLOAT3* const center, 
		const DirectX::XMFLOAT3* const forward,
		float backwardsDashDistance,
		float animationDuration
	);
	bool GetIsBackwardsDashAnimationComplete() const;
	bool GetIsRotationComplete() const;
	void SetIsRotationComplete(bool value);

private:
	// dash animation data
	DirectX::XMFLOAT3 mDashStartPosition = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 mDashTargetPosition = DirectX::XMFLOAT3(0, 0, 0);
	float mDashAnimationTimer = 0.5f;
	float mIsPerformingBackwardsDash = false;
	float mIsBackwardsDashAnimationComplete = true;
	DirectX::XMFLOAT3 mDashDirection = DirectX::XMFLOAT3(0, 0, 0);
	bool mIsRotationComplete = false;

private:
};