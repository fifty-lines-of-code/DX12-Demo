#pragma once

#include <array>
#include "../../../../Math/EngineMath.h"
#include "../../Resource Manager/ResourceManager.h"
#include "../../../Scene/SceneBlueprint.h"

namespace Engine::EngineWorld {

	class Chunk {
	public:
		Chunk();
		~Chunk();

		bool Initialize(
			uint16_t id, 
			Vector3& center,
			uint32_t chunkEntityStartIndex
		);

		bool Load(
			uint8_t* entityStartAddressInBytes, 
			EngineResources::ResourceManager& resourceManager,
			SceneBlueprint& sceneBlueprint
		);

		uint16_t GetID() const noexcept;

		uint16_t GetIdOfTerrainOrFloor() const noexcept;

	public:
		static constexpr uint8_t MAX_ENTITIES_IN_A_CHUNK = 64;

	private:
		// each chunk holds an array of entity indices
		// it's the responsibiity of the parents to ensure
		// correct ids are set in the correct chunks
		std::array<uint32_t, Chunk::MAX_ENTITIES_IN_A_CHUNK> mEntities;
		Vector3 mCenter;
		uint16_t mID;
		uint16_t mNextEntityID;
		uint16_t mIdOfTerrainOrFloor;
		uint8_t mTotalNumberOfEntities;

	private:
		bool Insert(uint32_t entityID);
	};
}