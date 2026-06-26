#include "Chunk.h"

#include "../../Entity/Entity.h"

namespace Engine::EngineWorld {

	Chunk::Chunk() :
		mID(-1), // will wrap around to max uint16_t
		mTotalNumberOfEntities(0),
		mIdOfTerrainOrFloor(-1), // sets to uint16_t max
		mNextEntityID(0)
	{}

	Chunk::~Chunk() {}

	bool Chunk::Initialize(uint16_t chunkID, Vector3& center, uint32_t chunkEntityStartIndex) {
		mID = chunkID;
		mCenter = center;
		mTotalNumberOfEntities = 0;
		mNextEntityID = chunkEntityStartIndex;

		return true;
	}

	bool Chunk::Load(
		uint8_t* entityStartAddressInBytes,
		EngineResources::ResourceManager& resourceManager,
		SceneBlueprint& sceneBlueprint
	) {
		// offset in bytes for this Entity
		uint32_t offsetForThisEntity = mNextEntityID * sizeof(Entity);

		// get address of first entity
		uint8_t* targetEntityAddress = entityStartAddressInBytes + offsetForThisEntity;

		for (uint32_t i = 0; i < sceneBlueprint.EntityCount; ++i) {
			const EntityBlueprint& entityBlueprint = sceneBlueprint.EntityBlueprints[i];
			if (entityBlueprint.EntityType == EntityType::PLAYER) {
				// skip all Player entities
				continue;
			}

			bool didUpdate = false;
			EngineResources::Mesh* mesh = nullptr;
			Entity& entity = *reinterpret_cast<Entity*>(targetEntityAddress);

			switch (entityBlueprint.EntityType) {
			case EntityType::TERRAIN: 
				// get pointer to the terrain mesh
				mesh = resourceManager.GetMesh(EngineResources::MeshID::TERRAIN_0x0);
				mIdOfTerrainOrFloor = mNextEntityID;
				break;
			case EntityType::FLOOR:
				mIdOfTerrainOrFloor = mNextEntityID;
				[[fallthrough]];
			case EntityType::WALL: 
				// get pointer to the terrain mesh
				mesh = resourceManager.GetMesh(EngineResources::MeshID::CUBE);
				break;
			case EntityType::MIRROR:
				mesh = resourceManager.GetMesh(EngineResources::MeshID::MIRROR);
				break;
			default: break;
			}

			if (mesh != nullptr) {
				entity.SetIsDirty(true);
				entity.SetIsActive(true);
				entity.SetID(mNextEntityID);
				entity.SetEntityType(entityBlueprint.EntityType);
				entity.SetCenter(entityBlueprint.Center);
				entity.SetScale(entityBlueprint.Scale);
				// set mesh
				entity.SetMesh(mesh);
				// update all submeshes
				for (int i = 0; i < entityBlueprint.ActiveSubMeshCount; ++i) {
					const EntitySubMeshBlueprint& subMeshBlueprint = entityBlueprint.EntitySubMeshBlueprints[i];
					entity.SetSubMeshMaterialAndTexture(
						i,
						subMeshBlueprint.MaterialType,
						subMeshBlueprint.TextureID
					);
				}

				if (!Insert(mNextEntityID)) { return false; }
				didUpdate = true;
			}

			if (didUpdate) {
				// go to next Entity by walking the Stride(Entity)
				targetEntityAddress += sizeof(Entity);
			}
		}

		return true;
	}

	uint16_t Chunk::GetID() const noexcept { return mID; }

	uint16_t Chunk::GetIdOfTerrainOrFloor() const noexcept {
		return mIdOfTerrainOrFloor; 
	}

#pragma region Private

	bool Chunk::Insert(uint32_t entityID) {
		// ensure we don't crash
		// false will clearly mean this chunk is full
		// so we either have a bug or rethink the scene or max_entities_in_a_chunk count

		if (mTotalNumberOfEntities >= Chunk::MAX_ENTITIES_IN_A_CHUNK) { return false; }

		mEntities[mTotalNumberOfEntities] = entityID;

		mTotalNumberOfEntities++;
		mNextEntityID++;

		return true;
	}

#pragma endregion
}