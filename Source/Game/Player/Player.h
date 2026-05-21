#pragma once

#include <DirectXMath.h>
#include ".././../Engine/Input System/IInputSystem.h"
#include "Player Animator/PlayerAnimator.h"
#include "Player Logic/PlayerLogic.h"

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
	PlayerLogic mPlayerLogic;
	PlayerAnimator mPlayerAnimator;

	DirectX::XMFLOAT3 mCenter = DirectX::XMFLOAT3(0.f, 0.6f, .5f);
	float mCurrentRotation = DirectX::XMConvertToRadians(0);

private:
	DirectX::XMFLOAT3 CalculateMovementVectorFrom(CameraForwardAndRightVectors forwardAndRightVectors, float leftStickX, float leftStickY);
	void UpdateEntityCenterAndRotationAndSetItToDirty();
};