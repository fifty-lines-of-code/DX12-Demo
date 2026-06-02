#pragma once

#include <array>
#include "Chunk/Chunk.h"

namespace Engine {

	class ChunksManager {
	public:
		ChunksManager();
		~ChunksManager();

		bool Initialize();

	private:
		static constexpr float CHUNK_SIZE = 32.0f;
		static constexpr size_t MAX_ACTIVE_CHUNKS = 1;

		std::array<Chunk, MAX_ACTIVE_CHUNKS> mActiveChunks;
		uint32_t mActiveChunkCount;

	private:
		bool LoadInitialChunks();

	};
}