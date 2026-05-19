#pragma once

#include <DirectXMath.h>
#include ".././../Engine/Input System/IInputSystem.h"

struct CameraForwardAndRightVectors;
class Entity;

class Player {
public:
	Player();
	~Player();

	void SetEntity(Entity* entity);
	void Update(float deltaTime, const IInputSystem* const inputSystem, CameraForwardAndRightVectors forwardAndRightVectors);

	DirectX::XMFLOAT4 GetCenter() const;

private:
	Entity* mEntity = nullptr;
	DirectX::XMFLOAT3 mCenter = DirectX::XMFLOAT3(0.f, 0.6f, .5f);
	float mCurrentRotation = DirectX::XMConvertToRadians(0);

	float mPlayerSlowWalkingSpeed = 0.2f;
	float mPlayerNormalWalkingSpeed = .375f;
	float mPlayerRunningSpeed = .65f;
	float mRotationSpeed = 9.21f;
	float mTotalTimeBWasHeldDown = -1; // negative means not held down
	const float mTotalDurationPlayerHasToRunAfterPressingB = 0.04;
	bool mIsRunning = false;
	bool mPerformBackwardsDashIfNotRunning = false;
	bool mIsPerformingBackwardsDash = false;
	float mBackwardsDashAnimationDuration = .85f;
	float mBackwardsDashDistance = 2.f;
	DirectX::XMFLOAT3 mDashStartPosition = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 mDashTargetPosition = DirectX::XMFLOAT3(0, 0, 0);
	float mDashAnimationTimer = 0.f;

private:
	void UpdateActionEastButtonState(float deltaTime, GameButtonState actionEastButtonState);
	void MoveAndRotatePlayer(float deltaTime, float leftStickX, float leftStickY, CameraForwardAndRightVectors forwardAndRightVectors);
	void RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movementVector);
	void UpdateEntityCenterAndRotationAndSetItToDirty();
};