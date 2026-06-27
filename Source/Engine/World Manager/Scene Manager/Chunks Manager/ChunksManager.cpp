#include "ChunksManager.h"
#include "../Entity/Entity.h"

namespace Engine::EngineWorld {

	bool ChunksManager::Initialize() {
		mNextChunkID = 0;

		if (!InitializeInitialChunks()) { return false; }
		// todo

		return true;
	}

	bool ChunksManager::Load(
		const std::vector<uint32_t>& entityIDs
	) noexcept {
		// todo:
		// multi thread loading of each of the initial (x) chunks 
		// for now we load a single central chunk
		return LoadChunkAtSlot0(entityIDs);
	}

	Vector2 ChunksManager::GetCenterXZOfChunkContaining(const Vector2& entityXZ) {
		// TODO:
		return Vector2(0.f);
	}

#pragma region Private

	bool ChunksManager::InitializeInitialChunks() {
		// todo:
		// multi thread loading of each of the initial (x) chunks 

		if (mNextChunkID >= MAX_CHUNKS) { 
			return false;
		}

		ChunkSlot &chunkSlotAt0 = mActiveChunks[mNextChunkID];

		// this chunk's center is 0, 0, 0
		// Vector3 initializes by default to 0, 0, 0
		Vector3 center;

		if (!chunkSlotAt0.Initialize(mNextChunkID, center)) {
			return false;
		}

		// explicitly update the next chunk ID
		mNextChunkID++;

		return true;
	}

	bool ChunksManager::LoadChunkAtSlot0(
		const std::vector<uint32_t>& entityIDs
	) noexcept {
		ChunkSlot& chunkSlotAt0 = mActiveChunks[0];

		chunkSlotAt0.LoadChunk(entityIDs);

		return true;
	}

#pragma endregion
}