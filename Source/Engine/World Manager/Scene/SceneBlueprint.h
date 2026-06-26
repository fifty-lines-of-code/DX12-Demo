#pragma once

#include <cstdint>
#include "../../Math/EngineMath.h"
#include "../Scene Manager/Entity/EntityType.h"
#include "../Scene Manager/Resource Manager/Materials Manager/Material/Material.h"
#include "../Scene Manager/Resource Manager/Texture Manager/TextureID.h"

namespace Engine::EngineWorld {

	enum class Scene : uint32_t {
		FLAT_PLANE,
		SINGLE_MIRROR,
		HEIGHTMAP,
		COUNT,
		INVALID
	};

	struct EntitySubMeshBlueprint {
		EngineResources::MaterialType MaterialType;
		EngineResources::TextureID TextureID;
	};

	struct EntityBlueprint {
		Vector3 Center;
		Vector3 Scale;
		BasisVectors BasisVectors;
		Vector3 SurfaceNormal;
		EntityType EntityType;
		EngineResources::MeshID MeshID;
		std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> EntitySubMeshBlueprints;
		uint8_t ActiveSubMeshCount;
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