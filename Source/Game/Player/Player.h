#pragma once

class Entity;
class IInputSystem;

class Player {
public:
	Player(Entity* entity);
	~Player();

	void Update(float deltaTime, const IInputSystem* const inputSystem);

private:
	Entity* mEntity = nullptr;
	float mPlayerMovementSpeed = .375f;

private:
	void MovePlayer(float deltaTime, float leftStickX, float leftStickY);
};