#pragma once

#include <array>
#include "../../../EngineConfig.h"

namespace Engine::EngineWorld {

	class ReflectionManager {
	public:
		ReflectionManager();
		~ReflectionManager() = default;

		bool Initialize();
		bool RegisterMirror(uint32_t mirrorEntityIndex);
		void Reset() noexcept;

	private:
		std::array<uint32_t, EngineConfig::EngineConfig::MAX_MIRROR_PLANES> mMirrorEntityIndices;
		uint32_t mMirrorCount;
	};
}