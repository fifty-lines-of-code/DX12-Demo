#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>

class Camera;
class Entity;
class ResourceManager;

class SceneManager {
public:
	SceneManager();
	~SceneManager();

	bool Initialize();
	bool LoadScene();
	void Update(DirectX::XMFLOAT4X4 viewProj);


	size_t GetEntityCount() const;
	size_t GetConstantBufferDataByteSizeOfEachEntity() const;

	const std::vector<std::unique_ptr<Entity>>& GetEntities() const;

private:
	std::vector<std::unique_ptr<Entity>> mEntities;
	std::unique_ptr<ResourceManager> mResourceManager;

private:
	bool GenerateCubeEntity();
};