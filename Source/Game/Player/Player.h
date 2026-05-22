#pragma once

#include "../../Helper/MathHelper.h"
#include <DirectXMath.h>
#include ".././../Engine/Input System/IInputSystem.h"
#include "Player Animator/PlayerAnimator.h"
#include "Player Logic/PlayerLogic.h"

struct BasisVectors;
class Entity;

class Player {
public:
	Player();
	~Player();

	void SetEntity(Entity* entity);
	void Update(float deltaTime, const IInputSystem* const inputSystem, BasisVectors forwardAndRightVectors);

	DirectX::XMFLOAT4 GetCenter() const;

private:
	Entity* mEntity = nullptr;
	PlayerLogic mPlayerLogic;
	PlayerAnimator mPlayerAnimator;

	DirectX::XMFLOAT3 mCenter = DirectX::XMFLOAT3(0.f, 0.6f, .5f);
	float mCurrentRotation = DirectX::XMConvertToRadians(0);

	// basis vectors
	BasisVectors mBasisVectors;

private:
	void UpdateForwardAndRightVectorsFromCurrentRotation();
	void CalculateMovementVector(
		float leftStickX,
		float leftStickY,
		BasisVectors basisVectors,
		DirectX::XMFLOAT3& movement
	);
	void UpdateEntityCenterAndRotationAndSetItToDirty();
};