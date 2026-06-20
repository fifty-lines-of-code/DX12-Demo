#pragma once

#include <cstdint>
#include "../../Math/EngineMath.h"
#include "../Scene Manager/Resource Manager/Materials Manager/Material/Material.h"
#include "../Scene Manager/Resource Manager/Texture Manager/TextureID.h"

namespace Engine::EngineWorld {

	enum class Scene : uint32_t {
		FLAT_PLAIN,
		MIRROR_DEMO,
		HEIGHTMAP,
		COUNT,
		INVALID
	};

	struct EntityBlueprint {
		uint32_t ID;
		Vector3 Center;
		Vector3 Scale;
		EngineResources::MaterialType MaterialType;
		EngineResources::TextureID TextureID;
		MeshID MeshID;
	};

	struct SceneBlueprint {
		Scene Scene;
		const EntityBlueprint* EntityBlueprints;
		uint32_t EntityCount;
		uint32_t SizeOfEntityBlueprint;
		Vector3 SunStrength;
		Vector3 SunDirection;
		// todo: add more light blueprints
	};
}