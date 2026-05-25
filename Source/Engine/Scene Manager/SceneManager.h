#pragma once

#include <memory>
#include "../Scene Manager/Entities/Mesh/Mesh.h"
#include "Resource Manager/ResourceManager.h"
#include <unordered_map>
#include <vector>

class Camera;
class Entity;
class IInputSystem;

class SceneManager {
public:
	SceneManager();
	~SceneManager();

	bool Initialize();
	bool LoadScene();
	void Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed);

	uint32_t GetEntityCount() const;
	uint32_t GetConstantBufferDataByteSizeOfEachEntity() const;
	uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const;

	const std::vector<std::unique_ptr<Entity>>* GetEntities() const;
	std::vector<const Mesh*> GetMeshesToLoad();
	Entity* GetPlayerEntity() const;

private:
	uint32_t mIDOfNextEntityThatWillBeCreated = 0;
	std::vector<std::unique_ptr<Entity>> mEntities;
	ResourceManager mResourceManager;
	std::unordered_map<MeshID, const Mesh*> mMeshesToLoad;

private:
	bool GeneratePlayerEntity();
	bool GenerateBasicScene();
};