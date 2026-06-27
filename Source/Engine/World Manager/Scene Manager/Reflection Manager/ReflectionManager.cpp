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

	void ReflectionManager::LoadMirrorPlaneQueryResult(
		EngineSimulation::MirrorPlaneQueryResult& result,
		const std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& entities
	) const noexcept {
		if (mMirrorCount == 0 ||
			mMirrorCount >= EngineConfig::EngineConfig::MAX_MIRROR_PLANES) {
			result.Count = 0;
			return;
		}
		result.Count = 0;

		for (uint32_t i = 0; i < mMirrorCount; ++i) {
			uint32_t entityIndex = mMirrorEntityIndices[i];
			if (entityIndex >= EngineConfig::EngineConfig::MAX_ENTITIES) { continue; }

			result.Count++;

			const Entity& entity = entities[entityIndex];

			Vector3 entityCenter = entity.GetCenter();
			Vector3 entityScale = entity.GetScale();
			Vector3 entityNormal = entity.GetSurfaceNormal();

			EngineSimulation::MirrorPlaneData& data = result.Planes[i];

			// 1. Get the thickness along the local Z axis
			float halfThickness = entityScale.z * 0.5f;

			// 2. Project the world center forward along the surface normal by the half-thickness

			data.Normal = entityNormal;
			data.Center = Vector3(
				entityCenter.x + (entityNormal.x * halfThickness),
				entityCenter.y + (entityNormal.y * halfThickness),
				entityCenter.z + (entityNormal.z * halfThickness)
			);
		}
	}

	void ReflectionManager::Reset() noexcept { mMirrorCount = 0; }

	bool ReflectionManager::HasActiveMirrors() const noexcept {
		return mMirrorCount > 0;
	}
}