#pragma once

#include "Chunk/Chunk.h"

namespace Engine::EngineWorld {

	struct SceneBlueprint;

	struct ChunkSlot {
	public:
		ChunkSlot() = default;
		~ChunkSlot() = default;

		ChunkSlot(const ChunkSlot& rhs) = delete;
		ChunkSlot& operator=(const ChunkSlot& rhs) = delete;
		ChunkSlot(ChunkSlot&&) = delete;
		ChunkSlot& operator=(ChunkSlot&&) = delete;

		bool Initialize(
			uint16_t chunkID,
			Vector3& chunkCenter
		) {
			if (isLoaded) { return false; }

			if (!mChunk.Initialize(chunkID, chunkCenter)) { return false; }

			return true;
		}

		bool LoadChunk(
			const std::vector<uint32_t>& entityIDs
		) {
			if (isLoaded) { return false; }

			bool loadResult = mChunk.Load(entityIDs);
			isLoaded = loadResult;

			return isLoaded;
		}

		uint16_t GetChunkID() { return mChunk.GetID(); }

		Chunk& GetChunk() noexcept { return mChunk; }

	private:
		Chunk mChunk;
		bool isLoaded = false;
	};
}