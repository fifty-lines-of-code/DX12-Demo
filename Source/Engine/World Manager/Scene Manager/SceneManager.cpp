#include "SceneManager.h"

#include "../../../Engine/Input System/IInputSystem.h"
#include "Resource Manager/ResourceManager.h"

namespace Engine {

	SceneManager::SceneManager() :
		mChunksManager(&mEntities[1]), // 0th index is always Player stored in scene manager
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


		// todo:
		// load the sun data from somewhere
		
		Vector3 strength = { 1.0f, 1.0f, 0.9f };
		Vector3 direction = { 0.577f, -0.577f, 0.577f };
		mLightsManager.Initialize(strength, direction);

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

	uint32_t SceneManager::GetEntityCount() const noexcept {
		return MAX_ENTITIES;
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

	std::array<Entity, SceneManager::MAX_ENTITIES>& SceneManager::GetEntities() {
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

	bool SceneManager::GeneratePlayerEntity() {
		Entity& playerEntity = mEntities[PLAYER_INDEX];
		playerEntity.SetIsActive(true);
		playerEntity.SetID(PLAYER_INDEX);
		playerEntity.GetPhysicsBody().Center = Vector3(-10.f, 0.875f, -10.5f);
		playerEntity.SetScale(Vector3(.75f, .75f, .75f));
		playerEntity.SetIsStatic(false);
		playerEntity.SetMaterialType(EngineResources::MaterialType::PLAYER);
		playerEntity.SetTextureID(EngineResources::TextureID::WOOD_CRATE);

		mIndexesOfDynamicEntities.push_back(PLAYER_INDEX);

		const Mesh* cubeMesh = mResourceManager.GetMesh(MeshID::Cube);
		playerEntity.SetMesh(cubeMesh);

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