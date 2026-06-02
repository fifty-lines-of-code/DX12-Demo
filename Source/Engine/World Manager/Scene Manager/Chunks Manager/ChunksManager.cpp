#include "ChunksManager.h"

namespace Engine {
	ChunksManager::ChunksManager() {

	}

	ChunksManager::~ChunksManager() {

	}

	bool ChunksManager::Initialize() {
		mActiveChunkCount = 0;

		if (!LoadInitialChunks()) { return false; }
		// todo

		return true;
	}

#pragma region Privte

	bool ChunksManager::LoadInitialChunks() {
		Chunk& chunkAt0 = mActiveChunks[mActiveChunkCount++];
		Vector3 center;

		if (!chunkAt0.Initialize(center)) { return false; }

		return true;
	}

#pragma endregion
}