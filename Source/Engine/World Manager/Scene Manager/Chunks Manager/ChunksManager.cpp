#include "ChunksManager.h"
#include "../Entity/Entity.h"

namespace Engine {
	ChunksManager::ChunksManager(void* entitiesStartAddress) :
		mEntitiesStartAddressInMemory(entitiesStartAddress),
		mNextChunkID(0),
		mActiveChunkCount(0)
	{}

	ChunksManager::~ChunksManager() {}

	bool ChunksManager::Initialize() {
		mActiveChunkCount = 0;

		if (!InitializeInitialChunks()) { return false; }
		// todo

		return true;
	}

	bool ChunksManager::LoadChunks(ResourceManager& resourceManager) {
		// todo:
		// multi thread loading of each of the initial (x) chunks 
		return LoadChunkAtSlot0(resourceManager);
	}

#pragma region Privte

	bool ChunksManager::InitializeInitialChunks() {
		// todo:
		// multi thread loading of each of the initial (x) chunks 

		if (mActiveChunkCount >= MAX_ACTIVE_CHUNKS ||
			mNextChunkID >= MAX_CHUNKS) { 
			return false;
		}

		ChunkSlot &chunkSlotAt0 = mActiveChunks[mActiveChunkCount++];
		// this chunk's center is 0, 0, 0
		Vector3 center;

		if (!chunkSlotAt0.Initialize(mNextChunkID++, center)) {
			return false;
		}

		return true;
	}

	bool ChunksManager::LoadChunkAtSlot0(ResourceManager& resourceManager) {
		ChunkSlot& chunkSlotAt0 = mActiveChunks[0];

		uint8_t* entityStartAddressInBytes = reinterpret_cast<uint8_t*>(const_cast<void*>(mEntitiesStartAddressInMemory));

		chunkSlotAt0.LoadChunk(entityStartAddressInBytes, resourceManager);

		return true;
	}

#pragma endregion
}