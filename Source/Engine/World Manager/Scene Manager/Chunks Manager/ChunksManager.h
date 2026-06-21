#pragma once

#include <array>
#include "ChunkSlot.h"
#include "../Resource Manager/ResourceManager.h"
#include "../../Scene/SceneBlueprint.h"
#include "../../WorldDimensions.h"

namespace Engine::EngineWorld {

	class ChunksManager {
	public:
		ChunksManager(
			void* mEntitiesStartAddressInMemory, 
			uint32_t chunkEntitiesStartingID
		);

		~ChunksManager();

		bool Initialize();
		bool LoadChunks(
			EngineResources::ResourceManager& resourceManager,
			SceneBlueprint& sceneBlueprint
		);
		Vector2 GetCenterXZOfChunkContaining(const Vector2& entityXZ);
		uint16_t GetIdOfTerrainOrFloor(
			float entityX, 
			float entityZ
		);

		static constexpr uint16_t CHUNK_SIZE = 32;
	private:
		static constexpr uint16_t MAX_CHUNKS_EACH_XZ_AXIS = WorldDimensions::World_Size / CHUNK_SIZE;
		static constexpr uint16_t MAX_CHUNKS = MAX_CHUNKS_EACH_XZ_AXIS * MAX_CHUNKS_EACH_XZ_AXIS;

		static constexpr size_t MAX_ACTIVE_CHUNKS = 1;

		std::array<ChunkSlot, MAX_ACTIVE_CHUNKS> mActiveChunks;
		const void* mEntitiesStartAddressInMemory;
		uint16_t mNextChunkID;
		uint32_t mChunkEntitiesStartingID;

	private:
		bool InitializeInitialChunks();
		bool LoadChunkAtSlot0(
			EngineResources::ResourceManager& resourceManager,
			SceneBlueprint& sceneBlueprint
		);
	};
}