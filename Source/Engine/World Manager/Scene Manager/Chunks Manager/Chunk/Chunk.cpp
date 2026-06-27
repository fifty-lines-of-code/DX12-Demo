#include "Chunk.h"

#include "../../Entity/Entity.h"

namespace Engine::EngineWorld {

	Chunk::Chunk() :
		mID(-1), // will wrap around to max uint16_t
		mIdOfTerrainOrFloor(-1), // sets to uint16_t max
		mEntityCount(0),
		mNextEntityIndex(0)
	{}

	bool Chunk::Initialize(
		uint16_t chunkID,
		Vector3& center
	) {
		mID = chunkID;
		mCenter = center;
		mEntityCount = 0;
		mNextEntityIndex = 0;

		return true;
	}

	bool Chunk::Load(
		const std::vector<uint32_t>& entityIDs
	) {
		for (const uint32_t entityID : entityIDs) {
			if (!Insert(entityID)) { return false; }
		}

		return true;
	}

	uint16_t Chunk::GetID() const noexcept { return mID; }

#pragma region Private

	bool Chunk::Insert(uint32_t entityID) {
		// ensure we don't crash
		// false will clearly mean this chunk is full
		// so we either have a bug or rethink the scene or max_entities_in_a_chunk count

		if (mEntityCount >= Chunk::MAX_ENTITIES_IN_A_CHUNK) { return false; }

		mEntities[mNextEntityIndex] = entityID;

		mEntityCount++;
		mNextEntityIndex++;

		return true;
	}

#pragma endregion
}