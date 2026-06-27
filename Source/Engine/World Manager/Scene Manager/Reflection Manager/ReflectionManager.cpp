#include "ReflectionManager.h"

#include "../../../../Helper/Helper.h"

namespace Engine::EngineWorld {

	ReflectionManager::ReflectionManager() : 
		mMirrorEntityIndices({}),
		mIsMirrorLUT({ false }),
		mMirrorCount(0)
	{}

	bool ReflectionManager::Initialize() {
		Reset();

		return true;
	}

	bool ReflectionManager::RegisterMirror(uint32_t mirrorEntityIndex)  {
		if (mMirrorCount >= EngineConfig::EngineConfig::MAX_MIRROR_PLANES ||
			mirrorEntityIndex >= EngineConfig::EngineConfig::MAX_ENTITIES) {
			return false;
		}

		mMirrorEntityIndices[mMirrorCount++] = mirrorEntityIndex;
		mIsMirrorLUT[mirrorEntityIndex] = true;

		return true;
	}

	void ReflectionManager::Reset() noexcept {
		mMirrorCount = 0;
		mIsMirrorLUT.fill(false);
	}

	bool ReflectionManager::HasActiveMirrors() const noexcept {
		return mMirrorCount > 0;
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

	bool ReflectionManager::IsMirrorEntity(uint32_t entityID) const noexcept {
		ENGINE_ASSERT(
			entityID < EngineConfig::EngineConfig::MAX_ENTITIES,
			L"Entity ID is invalid!!"
		);

		if (entityID >= EngineConfig::EngineConfig::MAX_ENTITIES) { return false; }

		return mIsMirrorLUT[entityID];
	}
}