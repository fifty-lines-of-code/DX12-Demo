#pragma once

#include <array>
#include "../../../../Math/EngineMath.h"
#include "../../Resource Manager/ResourceManager.h"

namespace Engine {

	class Chunk {
	public:
		Chunk();
		~Chunk();

		bool Initialize(uint16_t id, Vector3& center);
		bool Load(uint8_t* entityStartAddressInBytes, ResourceManager& resourceManager);
		uint16_t GetID() const noexcept;

	public:
		static constexpr uint8_t MAX_ENTITIES_IN_A_CHUNK = 64;

	private:
		// each chunk holds an array of entity indices
		// it's the responsibiity of the parents to ensure
		// correct ids are set in the correct chunks
		uint32_t mEntities[Chunk::MAX_ENTITIES_IN_A_CHUNK];
		Vector3 mCenter;
		uint16_t mID;
		uint16_t mNextEntityID;
		uint8_t mTotalNumberOfEntities;

	private:
		bool Insert(uint32_t entityID);
	};
}