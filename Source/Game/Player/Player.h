#pragma once

#include <DirectXMath.h>

class Entity;
class IInputSystem;

class Player {
public:
	Player();
	~Player();

	void SetEntity(Entity* entity);
	void Update(float deltaTime, const IInputSystem* const inputSystem);

	DirectX::XMFLOAT4 GetCenter() const;

private:
	Entity* mEntity = nullptr;
	DirectX::XMFLOAT3 mCenter = DirectX::XMFLOAT3(0.f, 0.6f, .5f);

	float mPlayerMovementSpeed = .375f;

private:
	void MovePlayer(float deltaTime, float leftStickX, float leftStickY);
	void UpdateEntityCenterAndSetItToDirty();
};