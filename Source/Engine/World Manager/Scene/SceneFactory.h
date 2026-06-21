#pragma once

#include <array>
#include "../../EngineConfig.h"
#include "../Scene Manager/Entity/Mesh/Mesh.h"
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
		bool UpdateBlueprint(
			uint32_t index,
			Vector3 center, 
			Vector3 scale, 
			EntityType entityType, 
			EngineResources::MaterialType materialType, 
			EngineResources::TextureID textureID,
			Engine::MeshID meshID
		);
	};
}