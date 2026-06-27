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

	private:
		std::array<uint32_t, EngineConfig::EngineConfig::MAX_MIRROR_PLANES> mMirrorEntityIndices;
		uint32_t mMirrorCount;
	};
}