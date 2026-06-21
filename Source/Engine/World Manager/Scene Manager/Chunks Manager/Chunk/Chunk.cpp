#include "Chunk.h"

#include "../../Entity/Entity.h"

namespace Engine::EngineWorld {

	Chunk::Chunk() :
		mID(-1), // will wrap around to max uint16_t
		mTotalNumberOfEntities(0),
		mNextEntityID(0)
	{}

	Chunk::~Chunk() {}

	bool Chunk::Initialize(uint16_t chunkID, Vector3& center) {
		mID = chunkID;
		mCenter = center;
		mTotalNumberOfEntities = 0;

		// +1 because player is always Player ID 0
		// todo: find a better way to do this
		mNextEntityID = mID * Chunk::MAX_ENTITIES_IN_A_CHUNK + 1;

		return true;
	}

	bool Chunk::Load(
		uint8_t* entityStartAddressInBytes,
		EngineResources::ResourceManager& resourceManager,
		SceneBlueprint& sceneBlueprint
	) {
		// first is always player, so no point in running this 
		// if there's only a single entity 
		// TODO: find a better way to handle this

		if (sceneBlueprint.EntityCount <= 1) { return false; }

		// calculate the offset
		uint32_t offsetForThisChunk = mID * (Chunk::MAX_ENTITIES_IN_A_CHUNK * sizeof(Entity));

		// -1 because player index starts at 1
		// and within this chunk we want a 0 - based indexing
		// todo: gotta find a better way to do this
		uint32_t offsetForThisEntity = offsetForThisChunk + ((mNextEntityID - 1) * sizeof(Entity));

		// get address of first entity
		uint8_t* targetEntityAddress = entityStartAddressInBytes + offsetForThisEntity;

		for (int i = 1; i < sceneBlueprint.EntityCount; ++i) {
			bool didUpdate = false;
			const Mesh* mesh = nullptr;
			const EntityBlueprint& entityBlueprint = sceneBlueprint.EntityBlueprints[i];

			switch (entityBlueprint.EntityType) {
			case EntityType::PLAYER:
				// we do nothing here
				break;
			case EntityType::TERRAIN: {
				// get pointer to the terrain mesh
				const Mesh* terrain0x0Mesh = resourceManager.GetMesh(MeshID::Terrain0x0);

				// update the entity that's now terrain
				Entity& terrain = *reinterpret_cast<Entity*>(targetEntityAddress);
				terrain.SetIsActive(true);
				terrain.SetID(mNextEntityID);
				terrain.GetPhysicsBody().Center = entityBlueprint.Center;
				terrain.SetScale(entityBlueprint.Scale);
				terrain.SetMesh(terrain0x0Mesh);
				terrain.SetMaterialType(entityBlueprint.MaterialType);
				terrain.SetTextureID(entityBlueprint.TextureID);
				terrain.SetIsDirty(true);

				if (!Insert(mNextEntityID)) { return false; }

				didUpdate = true;
				break;
			}
			case EntityType::WALL: {
				// get pointer to the cube mesh
				const Mesh* cubeMesh = resourceManager.GetMesh(MeshID::Cube);
				Entity& wall = *reinterpret_cast<Entity*>(targetEntityAddress);
				wall.SetIsActive(true);
				wall.SetID(mNextEntityID);
				wall.GetPhysicsBody().Center = entityBlueprint.Center;
				wall.SetScale(entityBlueprint.Scale);
				wall.SetMesh(cubeMesh);
				wall.SetMaterialType(entityBlueprint.MaterialType);
				wall.SetTextureID(entityBlueprint.TextureID);
				wall.SetIsDirty(true);

				if (!Insert(mNextEntityID)) { return false; }

				didUpdate = true;
				break;
			}
			}

			if (didUpdate) {
				// go to next Entity by walking the Stride(Entity)
				targetEntityAddress += sizeof(Entity);
			}
		}

		return true;
	}

	uint16_t Chunk::GetID() const noexcept { return mID; }

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