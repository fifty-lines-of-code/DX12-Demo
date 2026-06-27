#pragma once

#include <array>
#include "../../../EngineConfig.h"
#include "../Entity/Entity.h"
#include "../../../Simulation/SimulationDataStructures.h"

namespace Engine::EngineWorld {

	class ReflectionManager {
	public:
		ReflectionManager();
		~ReflectionManager() = default;

		ReflectionManager(const ReflectionManager& rhs) = delete;
		ReflectionManager& operator=(const ReflectionManager& rhs) = delete;
		ReflectionManager(ReflectionManager&&) = delete;
		ReflectionManager& operator=(ReflectionManager&&) = delete;

		bool Initialize();

		bool RegisterMirror(uint32_t mirrorEntityIndex);

		void Reset() noexcept;

		bool HasActiveMirrors() const noexcept;

		void LoadMirrorPlaneQueryResult(
			EngineSimulation::MirrorPlaneQueryResult& result,
			const std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& entities
		) const noexcept;

		bool IsMirrorEntity(uint32_t entityID) const noexcept;

	private:
		std::array<uint32_t, EngineConfig::EngineConfig::MAX_MIRROR_PLANES> mMirrorEntityIndices;
		// we want a O(1) lookup per frame when we query
		// if an entity is a mirror
		std::array<bool, EngineConfig::EngineConfig::MAX_ENTITIES> mIsMirrorLUT;
		uint32_t mMirrorCount;
	};
}