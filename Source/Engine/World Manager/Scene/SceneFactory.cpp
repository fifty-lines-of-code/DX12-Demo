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
		case Scene::FLAT_PLAIN:
			// todo;
			isLoaded = false;
			break;
		case Scene::MIRROR_DEMO:
			// todo;
			isLoaded = false;
			break;
		case Scene::HEIGHTMAP:
			if (!LoadHeightMapScene(sceneBlueprint)) {
				isLoaded = false;
			}
			else {
				isLoaded = true;
			}
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
		bool result;
		// Player entity
		result = UpdateBlueprint(
			0, // raw array index
			Vector3(-10.f, 0.875f, -10.5f),
			Vector3(.75f, .75f, .75f),
			EntityType::PLAYER,
			EngineResources::MaterialType::PLAYER,
			EngineResources::TextureID::WOOD_CRATE,
			MeshID::Cube
		);

		if (!result) { return false; }

		// up the count
		mCount++;

		// Terrain
		result = UpdateBlueprint(
			1, // raw array index
			Vector3(0.f, 0.f, 0.f),
			Vector3(Vector3(1.f)),
			EntityType::TERRAIN,
			EngineResources::MaterialType::TERRAIN,
			EngineResources::TextureID::INVALID,
			MeshID::Terrain0x0
		);

		if (!result) { return false; }

		mCount++;

		// Wall
		result = UpdateBlueprint(
			2, // raw array index
			Vector3(0.f, 4.1f, 3.f),
			Vector3(Vector3(1.5f, 2.f, .2f)),
			EntityType::WALL,
			EngineResources::MaterialType::WALL,
			EngineResources::TextureID::INVALID,
			MeshID::Cube
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

	bool SceneFactory::UpdateBlueprint(
		uint32_t index,
		Vector3 center,
		Vector3 scale,
		EntityType entityType,
		EngineResources::MaterialType materialType,
		EngineResources::TextureID textureID,
		MeshID meshID
	) {
		if (index >= EngineConfig::EngineConfig::MAX_ENTITIES) {
			Logger::ERR(L"Cannot load more Entities");
			return false;
		}

		EntityBlueprint& blueprintAtIndex = mBlueprintBackingMemory[index];
		blueprintAtIndex.Center = center;
		blueprintAtIndex.Scale = scale;
		blueprintAtIndex.EntityType = entityType;
		blueprintAtIndex.MaterialType = materialType;
		blueprintAtIndex.TextureID = textureID;
		blueprintAtIndex.MeshID = meshID;

		return true;
	}

#pragma endregion
}