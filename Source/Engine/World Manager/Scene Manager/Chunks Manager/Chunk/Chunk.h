#pragma once

#include <array>
#include "../../../../Math/EngineMath.h"
#include "../../Resource Manager/ResourceManager.h"
#include "../../../Scene/SceneBlueprint.h"

namespace Engine::EngineWorld {

	class Chunk {
	public:
		Chunk();
		~Chunk() = default;

		Chunk(const Chunk& rhs) = delete;
		Chunk& operator=(const Chunk& rhs) = delete;
		Chunk(Chunk&&) = delete;
		Chunk& operator=(Chunk&&) = delete;

		bool Initialize(
			uint16_t id, 
			Vector3& center
		);

		bool Load(
			const std::vector<uint32_t>& entityIDs
		);

		uint16_t GetID() const noexcept;

	public:
		static constexpr uint8_t MAX_ENTITIES_IN_A_CHUNK = 64;

	private:
		// each chunk holds an array of entity indices
		// it's the responsibiity of the parents to ensure
		// correct ids are set in the correct chunks
		std::array<uint32_t, Chunk::MAX_ENTITIES_IN_A_CHUNK> mEntities;
		Vector3 mCenter;
		uint16_t mID;
		uint16_t mEntityCount;
		uint16_t mNextEntityIndex;
		uint16_t mIdOfTerrainOrFloor;

	private:
		bool Insert(uint32_t entityID);
	};
}