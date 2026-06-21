#include "SceneManager.h"

#include "../../../Engine/Input System/IInputSystem.h"
#include "Resource Manager/ResourceManager.h"

namespace Engine::EngineWorld {

	SceneManager::SceneManager() :
		// Chunks manager entities id always start at 1
		// cause the player index is always 0
		mChunksManager(&mEntities[0], 1),
		mResourceManager(ChunksManager::CHUNK_SIZE)
	{}

	SceneManager::~SceneManager() {}

	bool SceneManager::Initialize(const Vector3& center, float halfWidth) {

		if (!mResourceManager.Initialize()) { return false; }

		// todo init chunk's manager from this center
		// so it loads the (x) chunks at and around this center
		// also gotta decide a good value for that (x)
		if (!mChunksManager.Initialize()) { return false; }

		if (!mOctTree.Initialize(halfWidth)) { return false; }

		if (!mLightsManager.Initialize()) { return false; }

		return true;
	}

	bool SceneManager::LoadScene(SceneBlueprint& blueprint) {
		// ALWAYS create Player Entity first so it has ID 0
		// todo: find a better way to enforce this
		if (!GeneratePlayerEntity(blueprint)) { return false; }

		// load the lights
		mLightsManager.Load(blueprint.SunStrength, blueprint.SunDirection);

		// load the chunks
		if (!mChunksManager.LoadChunks(mResourceManager, blueprint)) { return false; }

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
			if (index >= 0 && index < EngineConfig::EngineConfig::MAX_ENTITIES) {
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

	uint32_t SceneManager::GetEntityCount() const noexcept {
		return mEntityCount;
	}

	uint32_t SceneManager::GetMaterialCount() const noexcept {
		return mResourceManager.GetMaterialCount();
	}

	uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachEntity() const noexcept {
		return sizeof(EntityConstantBufferData);
	}

	uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachPerPassObject() const noexcept {
		return sizeof(PerPassConstantBufferData);
	}

	uint32_t SceneManager::GetConstantBufferDataByteSizeOfEachMaterialObject() const noexcept {
		return sizeof(EngineResources::MaterialData);
	}

	const Vector4& SceneManager::GetAmbientLight() const noexcept {
		return mLightsManager.GetAmbientLight();
	}

	void SceneManager::GetLightsData(LightsArray16& lights) const {
		lights[0] = mLightsManager.GetLightsData();
	}

	std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& SceneManager::GetEntities() {
		return mEntities;
	}

	EngineResources::MaterialArray& SceneManager::GetMaterials() noexcept {
		return mResourceManager.GetMaterials();
	}

	EngineResources::TextureArray& SceneManager::GetTextures() noexcept {
		return mResourceManager.GetTextures();
	}

	void SceneManager::PrepareForUpdate() {
		mOctTree.ClearDynamicEntities();
	}

#pragma region Private

	void SceneManager::GetMeshesToLoad(std::vector<const Mesh*>& meshes) {
		auto& meshesArray = mResourceManager.GetMeshes();
		meshes.reserve(meshesArray.size());

		for (const Mesh& mesh : meshesArray) {
			meshes.push_back(&mesh);
		}
	}

	Entity& SceneManager::GetPlayerEntity() {
		return mEntities[PLAYER_INDEX];
	}

	bool SceneManager::GeneratePlayerEntity(SceneBlueprint& blueprint) {
		if (blueprint.EntityCount == 0) { return false; }

		Entity& playerEntity = mEntities[PLAYER_INDEX];
		// currently player is always at index 0
		// TODO: find a better way to do this
		const EntityBlueprint& playerBlueprint = blueprint.EntityBlueprints[PLAYER_INDEX];
		playerEntity.SetIsActive(true);
		playerEntity.SetID(PLAYER_INDEX);
		playerEntity.GetPhysicsBody().Center = playerBlueprint.Center;
		playerEntity.SetScale(playerBlueprint.Scale);
		playerEntity.SetIsStatic(false);
		playerEntity.SetMaterialType(playerBlueprint.MaterialType);
		playerEntity.SetTextureID(playerBlueprint.TextureID);
		playerEntity.SetIsDirty(true);
		const Mesh* cubeMesh = mResourceManager.GetMesh(playerBlueprint.MeshID);
		playerEntity.SetMesh(cubeMesh);

		mIndexesOfDynamicEntities.push_back(PLAYER_INDEX);

		return true;
	}

	void SceneManager::PrepareForCollisionPass() {
		// add dynamic entities to the octtree
		for (uint32_t index : mIndexesOfDynamicEntities) {
			if (index >= 0 && index < EngineConfig::EngineConfig::MAX_ENTITIES) {
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