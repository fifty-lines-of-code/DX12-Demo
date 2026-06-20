#pragma once

#include "Chunk/Chunk.h"

namespace Engine::EngineWorld {

	struct SceneBlueprint;

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

		bool LoadChunk(
			uint8_t* entityStartAddressInBytes, 
			EngineResources::ResourceManager& resourceManager,
			SceneBlueprint& sceneBlueprint
		) {
			if (isLoaded) { return false; }

			bool loadResult = mChunk.Load(
				entityStartAddressInBytes,
				resourceManager,
				sceneBlueprint
			);
			isLoaded = loadResult;

			return isLoaded;
		}

		uint16_t GetChunkID() { return mChunk.GetID(); }

	private:
		Chunk mChunk;
		bool isLoaded = false;
	};
}