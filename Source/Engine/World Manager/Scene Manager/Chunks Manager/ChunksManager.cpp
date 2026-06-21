#include "ChunksManager.h"
#include "../Entity/Entity.h"

namespace Engine::EngineWorld {

	ChunksManager::ChunksManager(
		void* entitiesStartAddress,
		uint32_t chunkEntitiesStartingID
	) :
		mEntitiesStartAddressInMemory(entitiesStartAddress),
		mNextChunkID(0),
		mChunkEntitiesStartingID(chunkEntitiesStartingID),
		mActiveChunkCount(0)
	{}

	ChunksManager::~ChunksManager() {}

	bool ChunksManager::Initialize() {
		mActiveChunkCount = 0;

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

#pragma region Privte

	bool ChunksManager::InitializeInitialChunks() {
		// todo:
		// multi thread loading of each of the initial (x) chunks 

		if (mActiveChunkCount >= MAX_ACTIVE_CHUNKS ||
			mNextChunkID >= MAX_CHUNKS) { 
			return false;
		}

		ChunkSlot &chunkSlotAt0 = mActiveChunks[mActiveChunkCount];
		// this chunk's center is 0, 0, 0
		Vector3 center;

		// +1 because Player index is always 0
		// todo: find a better way to do this
		uint32_t chunkEntityStartIndex = 
			mChunkEntitiesStartingID + 
			mNextChunkID * Chunk::MAX_ENTITIES_IN_A_CHUNK;

		if (!chunkSlotAt0.Initialize(mNextChunkID, center, chunkEntityStartIndex)) {
			return false;
		}

		// explicitly update the next chunk ID and active chunk count
		mActiveChunkCount++;
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