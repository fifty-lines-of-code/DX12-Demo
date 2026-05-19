#pragma once

#include <DirectXMath.h>

struct CameraForwardAndRightVectors;
class Entity;
class IInputSystem;

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

	float mPlayerMovementSpeed = .375f;
	float mRotationSpeed = 9.21f;

private:
	void MoveAndRotatePlayer(float deltaTime, float leftStickX, float leftStickY, CameraForwardAndRightVectors forwardAndRightVectors);
	void RotatePlayer(float deltaTime, DirectX::XMFLOAT3 movementVector);
	void UpdateEntityCenterAndRotationAndSetItToDirty();
};