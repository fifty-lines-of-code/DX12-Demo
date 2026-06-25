#include "SceneManager.h"

#include "../../../Engine/Input System/IInputSystem.h"
#include "Resource Manager/ResourceManager.h"

namespace Engine::EngineWorld {

	SceneManager::SceneManager() :
		// Chunks manager entities id always start at 1
		// cause the player index is always 0
		// we pass in the start of the array
		mChunksManager(&mEntities[0], 1),
		mResourceManager(ChunksManager::CHUNK_SIZE),
		mEntityCount(0)
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
		// update entity count
		mEntityCount = blueprint.EntityCount;

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

	uint32_t SceneManager::GetConstantBufferDataByteSizeOFEntityPerSubMeshObject() const noexcept {
		return sizeof(EntitySubMeshConstantBufferData);
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

	float SceneManager::GetProposedYOfTerrainOrFloor(
		float entityX, 
		float entityZ,
		float deltaTime
	) {
		uint16_t terrainOrFloorId = mChunksManager.GetIdOfTerrainOrFloor(
			entityX,
			entityZ
		);

		if (terrainOrFloorId >= EngineConfig::EngineConfig::MAX_ENTITIES ||
			// wraps around to uint16_t max
			terrainOrFloorId == uint16_t(-1)) {
			return uint16_t(-1);
		}

		Entity& entity = mEntities[terrainOrFloorId];
		if (entity.GetIsTerrainOrFloor()) {
			switch (entity.GetEntityType()) {
			case EntityType::TERRAIN: {
				Vector2 entityXZ = Vector2(entityX, entityZ);
				Vector2 chunkCenterXZ = mChunksManager.GetCenterXZOfChunkContaining(
					entityXZ
				);

				return CalculateProposedYOfTerrain(
					*mEntities[terrainOrFloorId].GetMesh(),
					entityX,
					entityZ,
					chunkCenterXZ,
					deltaTime
				);
			}
			case EntityType::FLOOR:
				// add a small delta value (0.05f) so that
				// object appears just above the floor
				// TODO: extract this constant somewhere

				return entity.GetPhysicsBody().Scale.y * 0.5f + 0.05f;
			default:
				// it should never reach here, something has gone wrong
				return uint16_t(-1);
			}
		}
		else {
			// something went wrong this entity should be a terrain or a floor
			return uint16_t(-1);
		}
	}

#pragma region Private

	const EngineResources::MeshArray& SceneManager::GetMeshesToLoad() const noexcept {
		return mResourceManager.GetMeshes();
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
		playerEntity.SetIsDirty(true);
		playerEntity.SetIsActive(true);
		playerEntity.SetID(PLAYER_INDEX);
		playerEntity.SetEntityType(playerBlueprint.EntityType);
		playerEntity.GetPhysicsBody().Center = playerBlueprint.Center;
		playerEntity.SetScale(playerBlueprint.Scale);
		playerEntity.SetIsStatic(false);
		// set mesh
		EngineResources::Mesh* cubeMesh = mResourceManager.GetMesh(playerBlueprint.MeshID);
		playerEntity.SetMesh(cubeMesh);

		// update all submeshes
		for (int i = 0; i < playerBlueprint.ActiveSubMeshCount; ++i) {
			const EntitySubMeshBlueprint& subMeshBlueprint = playerBlueprint.EntitySubMeshBlueprints[i];
			playerEntity.SetSubMeshMaterialAndTexture(
				i,
				subMeshBlueprint.MaterialType,
				subMeshBlueprint.TextureID
			);
		}

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

	float SceneManager::CalculateProposedYOfTerrain(
		const EngineResources::Mesh& terrainMesh,
		float entityX,
		float entityZ,
		const Vector2& chunkCenterXZ,
		float deltaTime
	) {
		int density = (uint8_t)mResourceManager.GetTerrainLOD();

		float relativeToCenterX = entityX - chunkCenterXZ.x;
		float relativeToCenterZ = entityZ - chunkCenterXZ.y;

		// Subtracting the chunk's world center converts the entity's global position 
		// into local chunk coordinates, normalizing them into a 0 to CHUNK_SIZE range.
		float entityLocalX = relativeToCenterX + ChunksManager::CHUNK_SIZE / 2;
		float entityLocalZ = relativeToCenterZ + ChunksManager::CHUNK_SIZE / 2;

		uint32_t numberOfQuads = ChunksManager::CHUNK_SIZE * density;
		uint32_t numberOfVertices = numberOfQuads + 1;
		float vertexSpacing = 1.f / density;

		uint32_t x = (int)std::floor(entityLocalX / vertexSpacing);
		uint32_t z = (int)std::floor(entityLocalZ / vertexSpacing);

		if (x < 0 || x >= numberOfQuads ||
			z < 0 || z >= numberOfQuads) {
			// something went wrong, we are looking outside of this chunk
			return uint16_t(-1);
		}

		int bottomLeft = (z * numberOfVertices) + x;
		int bottomRight = bottomLeft + 1;
		int topLeft = bottomLeft + numberOfVertices;
		int topRight = topLeft + 1;

		const std::vector<Vertex>& vertices = terrainMesh.GetVertices();

		if (bottomLeft < 0 || bottomLeft >= vertices.size() ||
			bottomRight < 0 || bottomRight >= vertices.size() ||
			topLeft < 0 || topLeft >= vertices.size() ||
			topRight < 0 || topRight >= vertices.size()) {
			// something went wrong
			return uint16_t(-1);
		}
		float hBottomL = vertices[bottomLeft].Position.y;
		float hBottomR = vertices[bottomRight].Position.y;
		float hTopL = vertices[topLeft].Position.y;
		float hTopR = vertices[topRight].Position.y;

		// find fractional x
		float s = (entityLocalX / vertexSpacing) - std::floor(entityLocalX / vertexSpacing);
		// find fractional z
		float t = (entityLocalZ / vertexSpacing) - std::floor(entityLocalZ / vertexSpacing);

		float proposedY = hBottomL;

		// Determine which of the two triangles in the quad the player is standing on.
		if (t >= s) {
			// Triangle 1: bottomLeft -> topLeft -> topRight
			float dy = hTopL - hBottomL;
			float dx = hTopR - hTopL;
			proposedY = hBottomL + (t * dy) + (s * dx);
		}
		else {
			// Triangle 2: bottomLeft -> topRight -> bottomRight
			float dy = hTopR - hBottomR;
			float dx = hBottomR - hBottomL;
			proposedY = hBottomL + (t * dy) + (s * dx);
		}

		// add a tiny Y to elevate the entity above the terrain
		proposedY += 0.35f;

		return proposedY;
	}

#pragma endregion
}