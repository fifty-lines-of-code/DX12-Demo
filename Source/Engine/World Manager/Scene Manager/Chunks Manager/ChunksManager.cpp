#include "ChunksManager.h"
#include "../Entity/Entity.h"

namespace Engine::EngineWorld {

	ChunksManager::ChunksManager(
		void* entitiesStartAddress,
		uint32_t chunkEntitiesStartingID
	) :
		mEntitiesStartAddressInMemory(entitiesStartAddress),
		mNextChunkID(0),
		mChunkEntitiesStartingID(chunkEntitiesStartingID)
	{}

	ChunksManager::~ChunksManager() {}

	bool ChunksManager::Initialize() {
		mNextChunkID = 0;

		if (!InitializeInitialChunks()) { return false; }
		// todo

		return true;
	}

	bool ChunksManager::LoadChunks(
		EngineResources::ResourceManager& resourceManager,
		SceneBlueprint& sceneBlueprint
	) {
		// todo:
		// multi thread loading of each of the initial (x) chunks 
		// for now we load a single central chunk
		return LoadChunkAtSlot0(resourceManager, sceneBlueprint);
	}

	Vector2 ChunksManager::GetCenterXZOfChunkContaining(const Vector2& entityXZ) {
		// TODO:
		return Vector2(0.f);
	}

	uint16_t ChunksManager::GetIdOfTerrainOrFloor(float entityX, float entityZ) {
		// step 1: identify what chunk entity is in using x and z
		// TODO:
		// for now we only have a single chunk so we go straight to mActiveChunks[0]

		// step 2: Query that chunk slot for the id of the terrain or floor entity
		Chunk& chunk = mActiveChunks[0].GetChunk();
		return chunk.GetIdOfTerrainOrFloor();
	}

#pragma region Privte

	bool ChunksManager::InitializeInitialChunks() {
		// todo:
		// multi thread loading of each of the initial (x) chunks 

		if (mNextChunkID >= MAX_CHUNKS) { 
			return false;
		}

		ChunkSlot &chunkSlotAt0 = mActiveChunks[mNextChunkID];
		// this chunk's center is 0, 0, 0
		Vector3 center;

		uint32_t chunkEntityStartIndex = 
			mChunkEntitiesStartingID + 
			mNextChunkID * Chunk::MAX_ENTITIES_IN_A_CHUNK;

		if (!chunkSlotAt0.Initialize(mNextChunkID, center, chunkEntityStartIndex)) {
			return false;
		}

		// explicitly update the next chunk ID
		mNextChunkID++;

		return true;
	}

	bool ChunksManager::LoadChunkAtSlot0(
		EngineResources::ResourceManager& resourceManager,
		SceneBlueprint& sceneBlueprint
	) {
		ChunkSlot& chunkSlotAt0 = mActiveChunks[0];

		uint8_t* entityStartAddressInBytes = reinterpret_cast<uint8_t*>(const_cast<void*>(mEntitiesStartAddressInMemory));

		chunkSlotAt0.LoadChunk(
			entityStartAddressInBytes, 
			resourceManager, 
			sceneBlueprint
		);

		return true;
	}

#pragma endregion
}