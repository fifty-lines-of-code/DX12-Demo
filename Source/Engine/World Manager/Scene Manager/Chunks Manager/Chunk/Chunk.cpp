#include "Chunk.h"

#include "../../Entity/Entity.h"

namespace Engine {

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

		// +1 because player is always ID 0
		// todo: find a better way to do this
		mNextEntityID = mID * Chunk::MAX_ENTITIES_IN_A_CHUNK + 1;

		return true;
	}

	bool Chunk::Load(uint8_t* entityStartAddressInBytes, ResourceManager& resourceManager) {
		// calculate the offset
		uint32_t offsetForThisChunk = mID * (Chunk::MAX_ENTITIES_IN_A_CHUNK * sizeof(Entity));

		// -1 because player index starts at 1
		// and within this chunk we want a 0 - based indexing
		// todo: gotta find a better way to do this
		uint32_t offsetForThisEntity = offsetForThisChunk + ((mNextEntityID - 1) * sizeof(Entity));

		// get address of this entity
		uint8_t* targetEntityAddress = entityStartAddressInBytes + offsetForThisEntity;

		// get pointer to the terrain mesh
		const Mesh* terrain0x0Mesh = resourceManager.GetMesh(MeshID::Terrain0x0);

		// generate the terrain
		Entity& floor = *reinterpret_cast<Entity*>(targetEntityAddress);
		floor.SetIsActive(true);
		floor.SetID(mNextEntityID);
		floor.GetPhysicsBody().Center = Vector3(0.f, 0.f, 0.f);
		floor.SetScale(Vector3(1.f));
		floor.SetMesh(terrain0x0Mesh);

		if (!Insert(mNextEntityID)) { return false; }

		// generate the wall
		// go to next Entity by walking the Stride(Entity)
		targetEntityAddress += sizeof(Entity);

		// get pointer to the cube mesh
		const Mesh* cubeMesh = resourceManager.GetMesh(MeshID::Cube);
		Entity& wall = *reinterpret_cast<Entity*>(targetEntityAddress);
		wall.SetIsActive(true);

		wall.SetID(mNextEntityID);
		wall.GetPhysicsBody().Center = Vector3(0.f, 1.1f, 3.f);
		wall.SetScale(Vector3(1.5f, 2.f, .2f));
		wall.SetMesh(cubeMesh);

		if (!Insert(mNextEntityID)) { return false; }

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