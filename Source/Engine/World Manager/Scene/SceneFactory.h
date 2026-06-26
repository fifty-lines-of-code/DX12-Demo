#pragma once

#include <array>
#include "../../EngineConfig.h"
#include "../Scene Manager/Resource Manager/Mesh Generator/Mesh/Mesh.h"
#include "SceneBlueprint.h"

namespace Engine::EngineWorld {
	
	class SceneFactory {
	public:
		SceneFactory() = default;
		~SceneFactory() = default;

		void LoadScene(
			Scene scene,
			SceneBlueprint& sceneBlueprint, 
			bool& isLoaded
		);

	private:
		std::array<EntityBlueprint, EngineConfig::EngineConfig::MAX_ENTITIES> mBlueprintBackingMemory;
		uint32_t mCount = 0;

	private:
		void Reset();
		bool LoadHeightMapScene(SceneBlueprint& sceneBlueprint);
		bool LoadSingleMirroScene(SceneBlueprint& sceneBlueprint);
		bool UpdateBlueprint(
			uint32_t index,
			Vector3 center, 
			Vector3 scale, 
			EntityType entityType,
			EngineResources::MeshID meshID,
			std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshBlueprints,
			uint8_t activeSubMeshCount
		);
	};
}