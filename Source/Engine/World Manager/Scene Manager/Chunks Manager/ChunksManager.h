#pragma once

#include <array>
#include "ChunkSlot.h"
#include "../Resource Manager/ResourceManager.h"
#include "../../Scene/SceneBlueprint.h"
#include "../../WorldDimensions.h"

namespace Engine::EngineWorld {

	class ChunksManager {
	public:
		ChunksManager() = default;
		~ChunksManager() = default;

		ChunksManager(const ChunksManager& rhs) = delete;
		ChunksManager& operator=(const ChunksManager& rhs) = delete;
		ChunksManager(ChunksManager&&) = delete;
		ChunksManager& operator=(ChunksManager&&) = delete;

		bool Initialize();

		bool Load(
			const std::vector<uint32_t>& entityIDs
		) noexcept;
		Vector2 GetCenterXZOfChunkContaining(
			const Vector2& entityXZ
		);

		static constexpr uint16_t CHUNK_SIZE = 32;
	private:
		static constexpr uint16_t MAX_CHUNKS_EACH_XZ_AXIS = WorldDimensions::World_Size / CHUNK_SIZE;
		static constexpr uint16_t MAX_CHUNKS = MAX_CHUNKS_EACH_XZ_AXIS * MAX_CHUNKS_EACH_XZ_AXIS;

		static constexpr size_t MAX_ACTIVE_CHUNKS = 1;

		std::array<ChunkSlot, MAX_ACTIVE_CHUNKS> mActiveChunks;
		uint16_t mNextChunkID;

	private:
		bool InitializeInitialChunks();
		bool LoadChunkAtSlot0(
			const std::vector<uint32_t>& entityIDs
		) noexcept;
	};
}