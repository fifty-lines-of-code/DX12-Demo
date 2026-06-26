#pragma once

#include <array>
#include "../EngineConfig.h"
#include "../Math/Vector.h"

namespace Engine::EngineSimulation {

	struct MirrorPlaneData {
		Vector3 Normal;
		Vector3 Center;
	};

	struct MirrorPlaneQueryResult {
		std::array<MirrorPlaneData, EngineConfig::EngineConfig::MAX_MIRROR_PLANES> Planes;
		uint32_t Count;
	};
}