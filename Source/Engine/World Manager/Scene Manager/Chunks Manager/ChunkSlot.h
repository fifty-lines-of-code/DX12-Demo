#pragma once

#include "Chunk/Chunk.h"

namespace Engine {

	struct ChunkSlot {
	public:
		bool Initialize(
			uint16_t chunkID,
			Vector3& chunkCenter
		) {
			if (isLoaded) { return false; }

			if (!mChunk.Initialize(chunkID, chunkCenter)) { return false; }

			return true;
		}

		bool LoadChunk(uint8_t* entityStartAddressInBytes, EngineResources::ResourceManager& resourceManager) {
			if (isLoaded) { return false; }

			if (!mChunk.Load(entityStartAddressInBytes, resourceManager)) { return false; }

			isLoaded = true;

			return true;
		}

		uint16_t GetChunkID() { return mChunk.GetID(); }

	private:
		Chunk mChunk;
		bool isLoaded = false;
	};
}