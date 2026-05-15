#include "SceneManager.h"
#include "Entities/Entity.h"
#include "Resource Manager/ResourceManager.h"

SceneManager::SceneManager() :
	mResourceManager(std::make_unique<ResourceManager>()) {
}

SceneManager::~SceneManager() {
}

bool SceneManager::Initialize() {
	//todo: maybe some stuff here
	return true;
}

void SceneManager::Update(DirectX::XMFLOAT4X4 viewProj) {
	for (auto& entity : mEntities) {
		entity->Update(viewProj);
	}
}

size_t SceneManager::GetEntityCount() const {
	return mEntities.size();
}

size_t SceneManager::GetConstantBufferDataByteSizeOfEachEntity() const {
	return sizeof(EntityConstantBufferData);
}

const std::vector<std::unique_ptr<Entity>>& SceneManager::GetEntities() const {
	return mEntities;
}

bool SceneManager::LoadScene() {
	return GenerateCubeEntity();
}

bool SceneManager::GenerateCubeEntity() {
	std::unique_ptr<Entity> cubeEntity = std::make_unique<Entity>();
	cubeEntity->SetMesh(mResourceManager->GetMesh(MeshID::Cube));

	mEntities.push_back(std::move(cubeEntity));

	return true;
}