#include "SceneManager.h"

#include "Entities/Entity.h"
#include "../../Engine/Input System/IInputSystem.h"
#include "Resource Manager/ResourceManager.h"

SceneManager::SceneManager() :
	mResourceManager(std::make_unique<ResourceManager>()) 
{}

SceneManager::~SceneManager() {}

bool SceneManager::Initialize() {
	//todo: maybe some stuff here
	return true;
}

bool SceneManager::LoadScene() {
	// ALWAYS create Player Entity first so it has ID 0
	// todo: find a better way to enforce this
	if (!GeneratePlayerEntity()) { return false; }
	if (!GenerateBasicScene()) { return false; }

	return true;
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

bool SceneManager::GeneratePlayerEntity() {
	// Player Entity is ALWAYS 0
	// todo: find a better way to enforce this

	DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0.f, 0.6f, .5f);
	std::unique_ptr<Entity> playerEntity = std::make_unique<Entity>(0, center, 1.f, 1.f, 1.f);
	mIDOfNextEntityThatWillBeCreated = 1;

	const Mesh* cubeMesh = mResourceManager->GetMesh(MeshID::Cube);
	playerEntity->SetMesh(cubeMesh);

	mMeshesToLoad[cubeMesh->meshID] = cubeMesh;
	mEntities.push_back(std::move(playerEntity));

	return true;
}

bool SceneManager::GenerateBasicScene() {
	// generate the floor
	DirectX::XMFLOAT3 floorCenter = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	std::unique_ptr<Entity> floorEntity = std::make_unique<Entity>(
		mIDOfNextEntityThatWillBeCreated++,
		floorCenter,
		10.f,
		.2f,
		10.f
	);
	
	const Mesh* cubeMesh = mResourceManager->GetMesh(MeshID::Cube);
	floorEntity->SetMesh(cubeMesh);

	mEntities.push_back(std::move(floorEntity));

	// generate the wall
	DirectX::XMFLOAT3 wallCenter = DirectX::XMFLOAT3(0.f, 1.1f, 3.f);
	std::unique_ptr<Entity> wallEntity = std::make_unique<Entity>(
		mIDOfNextEntityThatWillBeCreated++,
		wallCenter,
		1.5f,
		2.f,
		.2f
	);

	wallEntity->SetMesh(cubeMesh);

	mEntities.push_back(std::move(wallEntity));

	return true;
}