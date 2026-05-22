#pragma once

#include <DirectXMath.h>
#include ".././../Engine/Input System/IInputSystem.h"
#include "Player Animator/PlayerAnimator.h"
#include "Player Logic/PlayerLogic.h"

struct CameraBasisVectors;
class Entity;

class Player {
public:
	Player();
	~Player();

	void SetEntity(Entity* entity);
	void Update(float deltaTime, const IInputSystem* const inputSystem, CameraBasisVectors forwardAndRightVectors);

	DirectX::XMFLOAT4 GetCenter() const;

private:
	Entity* mEntity = nullptr;
	PlayerLogic mPlayerLogic;
	PlayerAnimator mPlayerAnimator;

	DirectX::XMFLOAT3 mCenter = DirectX::XMFLOAT3(0.f, 0.6f, .5f);
	float mCurrentRotation = DirectX::XMConvertToRadians(0);

	// basis vectors
	DirectX::XMFLOAT3 mForward = DirectX::XMFLOAT3(0.f, 0.f, 1.f);
	DirectX::XMFLOAT3 mRight = DirectX::XMFLOAT3(1.f, 0.f, 0.f);
	DirectX::XMFLOAT3 mUp = DirectX::XMFLOAT3(0.f, 1.f, 0.f);

private:
	void UpdateForwardAndRightVectorsFromCurrentRotation();
	void CalculateMovementVector(
		float leftStickX,
		float leftStickY,
		CameraBasisVectors basisVectors,
		DirectX::XMFLOAT3& movement
	);
	void UpdateEntityCenterAndRotationAndSetItToDirty();
};