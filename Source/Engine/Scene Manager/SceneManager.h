#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <unordered_map>
#include "../Scene Manager/Entities/Mesh/Mesh.h"

class Camera;
class Entity;
class IInputSystem;
class ResourceManager;

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

private:
	uint32_t mIDOfNextEntityThatWillBeCreated = 0;
	std::vector<std::unique_ptr<Entity>> mEntities;
	std::unique_ptr<ResourceManager> mResourceManager;
	std::unordered_map<MeshID, const Mesh*> mMeshesToLoad;

private:
	bool GenerateCubeEntity();
};