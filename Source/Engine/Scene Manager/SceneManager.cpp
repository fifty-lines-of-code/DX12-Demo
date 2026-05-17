#include "SceneManager.h"

#include "Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"
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

bool SceneManager::LoadScene() {
	return GenerateCubeEntity();
}

void SceneManager::Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed) {
	for (const auto& entity : mEntities) {
		entity->Update(
			inputSystem->GetLeftStickX(),
			inputSystem->GetLeftStickY(),
			deltaTime,
			animationSpeed
		);
	}
}

uint32_t SceneManager::GetEntityCount() const {
	return (uint32_t)mEntities.size();
}

uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachEntity() const {
	return sizeof(EntityConstantBufferData);
}

uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachPerPassObject() const {
	return sizeof(PerPassConstantBufferData);
}

const std::vector<std::unique_ptr<Entity>>* SceneManager::GetEntities() const {
	return &mEntities;
}

std::vector<const Mesh*> SceneManager::GetMeshesToLoad() {
	std::vector<const Mesh*> meshList;
	meshList.reserve(mMeshesToLoad.size());

	for (auto const& pair : mMeshesToLoad) {
		meshList.push_back(pair.second);
	}

	return meshList;
}

Entity* SceneManager::GetPlayerEntity() const {
	if (mEntities.size() == 0) { return nullptr; }

	// Player entities index is always 0
	return mEntities[0].get();
}

bool SceneManager::GenerateCubeEntity() {
	std::unique_ptr<Entity> cubeEntity = std::make_unique<Entity>(mIDOfNextEntityThatWillBeCreated++);
	const Mesh* cubeMesh = mResourceManager->GetMesh(MeshID::Cube);
	cubeEntity->SetMesh(cubeMesh);
	mMeshesToLoad[cubeMesh->meshID] = cubeMesh;

	mEntities.push_back(std::move(cubeEntity));

	return true;
}