#include "SceneManager.h"

#include "../../../Engine/Input System/IInputSystem.h"
#include "Resource Manager/ResourceManager.h"

namespace Engine {

	SceneManager::SceneManager() :
		mResourceManager(ResourceManager()),
		mOctTree(OctTree())
	{}

	SceneManager::~SceneManager() {}

	bool SceneManager::Initialize(const Vector3& center, float halfWidth) {
		if (!mOctTree.Initialize(center, halfWidth)) { return false; }

		return true;
	}

	bool SceneManager::LoadScene() {
		// ALWAYS create Player Entity first so it has ID 0
		// todo: find a better way to enforce this
		if (!GeneratePlayerEntity()) { return false; }
		if (!GenerateBasicScene()) { return false; }

		// only add static entities to the OctTree during Load
		for (const auto& entity : mEntities) {
			if (entity.GetIsStatic()) { 
				mOctTree.Insert(entity.GetID(), entity.GetAABB(), true); 
			}
		}

		return true;
	}

	void SceneManager::GetPotentialCollisionsWithAABB(
		const AABB& playerPotentialAABB,
		std::vector<const Entity*>& candidates
	) {
		// prepare for collision checks
		PrepareForCollisionPass();

		std::vector<uint32_t> candidateIndexes;
		// find all entities player may be colliding with
		mOctTree.GetPotentialCollisionsWithAABB(
			PLAYER_INDEX,
			playerPotentialAABB,
			candidateIndexes
		);

		for (uint32_t index : candidateIndexes) {
			if (index >= 0 && index < MAX_ENTITIES) {
				candidates.push_back(&mEntities[index]);
			}
		}
	}

	void SceneManager::Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed) {

		// tell entities to update their World matrix
		for (auto& entity : mEntities) {
			entity.Update(
				inputSystem->GetLeftStickX(),
				inputSystem->GetLeftStickY(),
				deltaTime,
				animationSpeed
			);
		}
	}

	uint32_t SceneManager::GetEntityCount() const {
		return MAX_ENTITIES;
	}

	uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachEntity() const {
		return sizeof(EntityConstantBufferData);
	}

	uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachPerPassObject() const {
		return sizeof(PerPassConstantBufferData);
	}

	std::array<Entity, SceneManager::MAX_ENTITIES>& SceneManager::GetEntities() {
		return mEntities;
	}

	void SceneManager::PrepareForUpdate() {
		mOctTree.ClearDynamicEntities();
	}

#pragma region Private

	std::vector<const Mesh*> SceneManager::GetMeshesToLoad() {
		std::vector<const Mesh*> meshList;
		meshList.reserve(mMeshesToLoad.size());

		for (auto const& pair : mMeshesToLoad) {
			meshList.push_back(pair.second);
		}

		return meshList;
	}

	Entity& SceneManager::GetPlayerEntity() {
		return mEntities[PLAYER_INDEX];
	}

	bool SceneManager::GeneratePlayerEntity() {
		Entity& playerEntity = mEntities[PLAYER_INDEX];
		playerEntity.SetID(PLAYER_INDEX);
		playerEntity.GetPhysicsBody().Center = Vector3(0.f, 0.65f, 0.5f);
		playerEntity.SetScale(Vector3(1.f, 1.f, 1.f));
		playerEntity.SetIsStatic(false);
		mIndexesOfDynamicEntities.push_back(PLAYER_INDEX);

		const Mesh* cubeMesh = mResourceManager.GetMesh(MeshID::Cube);
		playerEntity.SetMesh(cubeMesh);

		mMeshesToLoad[cubeMesh->GetMeshID()] = cubeMesh;

		++mIDOfNextEntityThatWillBeCreated;

		return true;
	}

	bool SceneManager::GenerateBasicScene() {
		// generate the floor
		Entity& floor = mEntities[mIDOfNextEntityThatWillBeCreated];
		floor.SetID(mIDOfNextEntityThatWillBeCreated);
		floor.GetPhysicsBody().Center = Vector3(0.f, 0.f, 0.f);
		floor.SetScale(Vector3(10.f, .2f, 10.f));

		const Mesh* cubeMesh = mResourceManager.GetMesh(MeshID::Cube);
		floor.SetMesh(cubeMesh);

		++mIDOfNextEntityThatWillBeCreated;

		// generate the wall
		Entity& wall = mEntities[mIDOfNextEntityThatWillBeCreated];
		wall.SetID(mIDOfNextEntityThatWillBeCreated);
		wall.GetPhysicsBody().Center = Vector3(0.f, 1.1f, 3.f);
		wall.SetScale(Vector3(1.5f, 2.f, .2f));

		wall.SetMesh(cubeMesh);

		++mIDOfNextEntityThatWillBeCreated;

		return true;
	}

	void SceneManager::PrepareForCollisionPass() {
		// add dynamic entities to the octtree
		for (uint32_t index : mIndexesOfDynamicEntities) {
			if (index >= 0 && index < MAX_ENTITIES) {
				Entity& entity = mEntities[index];

				mOctTree.Insert(
					entity.GetID(),
					entity.GetAABB(),
					false
				);
			}
		}
	}

#pragma endregion
}