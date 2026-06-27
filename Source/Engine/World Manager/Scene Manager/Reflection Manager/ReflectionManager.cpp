#include "ReflectionManager.h"

namespace Engine::EngineWorld {

	ReflectionManager::ReflectionManager() : 
		mMirrorEntityIndices({}),
		mMirrorCount(0)
	{}

	bool ReflectionManager::Initialize() {
		Reset();

		return true;
	}

	bool ReflectionManager::RegisterMirror(uint32_t mirrorEntityIndex) {
		if (mMirrorCount >= EngineConfig::EngineConfig::MAX_MIRROR_PLANES) {
			return false;
		}

		mMirrorEntityIndices[mMirrorCount++] = mirrorEntityIndex;

		return true;
	}

	void ReflectionManager::Reset() noexcept { mMirrorCount = 0; }
}