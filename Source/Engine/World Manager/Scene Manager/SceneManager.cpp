#include "SceneManager.h"

#include "../../../Engine/Input System/IInputSystem.h"
#include "Resource Manager/ResourceManager.h"

namespace Engine {

	SceneManager::SceneManager() :
		mChunksManager(&mEntities[1]) // 0th index is always Player stored in scene manager
	{}

	SceneManager::~SceneManager() {}

	bool SceneManager::Initialize(const Vector3& center, float halfWidth) {
		if (!mOctTree.Initialize(halfWidth)) { return false; }

		// todo init chunk's manager from this center
		// so it loads the (x) chunks at and around this center
		// also gotta decide a good value for that (x)

		if (!mChunksManager.Initialize()) { return false; }

		return true;
	}

	bool SceneManager::LoadScene() {
		// ALWAYS create Player Entity first so it has ID 0
		// todo: find a better way to enforce this
		if (!GeneratePlayerEntity()) { return false; }

		// load the chunks
		if (!mChunksManager.LoadChunks(mResourceManager)) { return false; }

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

	void SceneManager::GetMeshesToLoad(std::vector<const Mesh*>& meshes) {
		meshes.reserve(mMeshesToLoad.size());

		for (auto const& pair : mMeshesToLoad) {
			meshes.push_back(pair.second);
		}
	}

	Entity& SceneManager::GetPlayerEntity() {
		return mEntities[PLAYER_INDEX];
	}

	bool SceneManager::GeneratePlayerEntity() {
		Entity& playerEntity = mEntities[PLAYER_INDEX];
		playerEntity.SetIsActive(true);
		playerEntity.SetID(PLAYER_INDEX);
		playerEntity.GetPhysicsBody().Center = Vector3(0.f, 0.65f, 0.5f);
		playerEntity.SetScale(Vector3(1.f, 1.f, 1.f));
		playerEntity.SetIsStatic(false);
		mIndexesOfDynamicEntities.push_back(PLAYER_INDEX);

		const Mesh* cubeMesh = mResourceManager.GetMesh(MeshID::Cube);
		playerEntity.SetMesh(cubeMesh);

		mMeshesToLoad[cubeMesh->GetMeshID()] = cubeMesh;

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