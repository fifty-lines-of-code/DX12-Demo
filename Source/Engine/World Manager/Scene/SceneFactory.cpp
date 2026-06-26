#include "SceneFactory.h"

#include "../../../Helper/Logger.h"

namespace Engine::EngineWorld {

	void SceneFactory::LoadScene(
		Scene scene,
		SceneBlueprint& sceneBlueprint,
		bool& isLoaded
	) {
		Reset();

		switch (scene) {
		case Scene::HEIGHTMAP:
			isLoaded = LoadHeightMapScene(sceneBlueprint);
			break;
		case Scene::FLAT_PLANE:
			// todo;
			ENGINE_ASSERT(false, L" Flat Plane Scene Still needs to be implemented");
			break;
		case Scene::SINGLE_MIRROR:
			isLoaded = LoadSingleMirroScene(sceneBlueprint);
			break;
		default:
			isLoaded = false;
		}
	}

#pragma region Private

	void SceneFactory::Reset() {
		mCount = 0;
	}

	bool SceneFactory::LoadHeightMapScene(SceneBlueprint& blueprint) {
		bool result = false;

		std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshBlueprints = {};

		// Player entity
		EntitySubMeshBlueprint& subMeshBlueprint0 = subMeshBlueprints[0];
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::PLAYER;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::WOOD_CRATE;

		result = UpdateBlueprint(
			0, // raw array index
			Vector3(-10.f, 0.875f, -10.5f), // center
			Vector3(.75f, .75f, .75f), // scale
			EntityType::PLAYER, // entitytype
			EngineResources::MeshID::CUBE, // mesh id,
			subMeshBlueprints, // submesh blueprints,
			1 // active submesh count
		);

		if (!result) { return false; }

		// up the count
		mCount++;

		// Terrain
		subMeshBlueprint0 = {};
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::TERRAIN;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::INVALID;

		result = UpdateBlueprint(
			1, // raw array index
			Vector3(0.f, 0.f, 0.f),
			Vector3(1.f),
			EntityType::TERRAIN,
			EngineResources::MeshID::TERRAIN_0x0,
			subMeshBlueprints,
			1
		);

		if (!result) { return false; }

		mCount++;

		// Wall
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::WALL;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::INVALID;
		result = UpdateBlueprint(
			2, // raw array index
			Vector3(0.f, 4.1f, -1.f), // center
			Vector3(1.5f, 2.f, .2f),
			EntityType::WALL,
			EngineResources::MeshID::CUBE,
			subMeshBlueprints,
			1
		);

		if (!result) { return false; }

		mCount++;

		// load lights
		Vector3 strength = { 1.0f, 1.0f, 0.9f };
		Vector3 direction = { 0.577f, -0.577f, 0.577f };

		// update the blueprint
		blueprint.Scene = Scene::HEIGHTMAP;
		blueprint.EntityBlueprints = mBlueprintBackingMemory.data();
		blueprint.EntityCount = mCount;
		blueprint.SizeOfEntityBlueprint = sizeof(EntityBlueprint);
		blueprint.SunStrength = strength;
		blueprint.SunDirection = direction;

		return true;
	}

	bool SceneFactory::LoadSingleMirroScene(SceneBlueprint& blueprint) {
		bool result = false;

		std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshBlueprints;

		// Player entity
		EntitySubMeshBlueprint& subMeshBlueprint0 = subMeshBlueprints[0];
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::PLAYER;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::WOOD_CRATE;

		result = UpdateBlueprint(
			0, // raw array index
			Vector3(-10.f, 0.975f, -10.5f), // center
			Vector3(.75f, .75f, .75f), // scale
			EntityType::PLAYER, // entitytype
			EngineResources::MeshID::CUBE, // mesh id,
			subMeshBlueprints, // submesh blueprints,
			1 // active submesh count
		);

		if (!result) { return false; }

		// up the count
		mCount++;

		// Floor
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::WALL;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::CHECKBOARD;
		result = UpdateBlueprint(
			1, // raw array index
			Vector3(0.f),
			Vector3(32.f, 0.2f, 32.f),
			EntityType::FLOOR,
			EngineResources::MeshID::CUBE,
			subMeshBlueprints,
			1
		);

		if (!result) { return false; }

		mCount++;

		// Mirror
		subMeshBlueprint0 = {};
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::WALL;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::INVALID;
		EntitySubMeshBlueprint& subMeshBlueprint1 = subMeshBlueprints[1];
		subMeshBlueprint1 = {};
		subMeshBlueprint1.MaterialType = EngineResources::MaterialType::MIRROR;
		subMeshBlueprint1.TextureID = EngineResources::TextureID::CHECKBOARD;
		result = UpdateBlueprint(
			2, // raw array index
			Vector3(0.f, 2.225f, -1.f), // center
			Vector3(1.5f, 2.f, .2f),
			EntityType::MIRROR,
			EngineResources::MeshID::MIRROR,
			subMeshBlueprints,
			2 // mirror has 2 submeshses
			// todo: find a better way to know these things
		);

		if (!result) { return false; }

		mCount++;

		// load lights
		Vector3 strength = { 1.0f, 1.0f, 0.9f };
		Vector3 direction = { 0.577f, -0.577f, 0.577f };

		// update the blueprint
		blueprint.Scene = Scene::SINGLE_MIRROR;
		blueprint.EntityBlueprints = mBlueprintBackingMemory.data();
		blueprint.EntityCount = mCount;
		blueprint.SizeOfEntityBlueprint = sizeof(EntityBlueprint);
		blueprint.SunStrength = strength;
		blueprint.SunDirection = direction;

		return true;
	}

	bool SceneFactory::UpdateBlueprint(
		uint32_t index,
		Vector3 center,
		Vector3 scale,
		EntityType entityType,
		EngineResources::MeshID meshID,
		std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshBlueprints,
		uint8_t activeSubMeshCount
	) {
		if (index >= EngineConfig::EngineConfig::MAX_ENTITIES) {
			Logger::ERR(L"Cannot load more Entities");
			return false;
		}

		EntityBlueprint& blueprintAtIndex = mBlueprintBackingMemory[index];
		blueprintAtIndex.Center = center;
		blueprintAtIndex.Scale = scale;
		blueprintAtIndex.EntityType = entityType;
		blueprintAtIndex.MeshID = meshID;
		blueprintAtIndex.EntitySubMeshBlueprints = subMeshBlueprints;
		blueprintAtIndex.ActiveSubMeshCount = activeSubMeshCount;

		return true;
	}

#pragma endregion
}