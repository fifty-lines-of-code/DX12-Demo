#include "SceneFactory.h"

#include <DirectXMath.h>
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
		subMeshBlueprint0.TextureID = EngineResources::TextureID::WOOD_CRATE_01;

		// calculate the basis vectors
		BasisVectors basisVectors;
		GenerateBasisVectorsFrom(
			0,
			0,
			0,
			basisVectors
		);

		result = UpdateBlueprint(
			0, // raw array index
			Vector3(-10.f, 0.875f, -10.5f), // center
			Vector3(.75f, .75f, .75f), // scale
			basisVectors, // basis vectors
			basisVectors.Forward, // surface normal
			EntityType::PLAYER, // entitytype
			EngineResources::MeshID::CUBE, // mesh id,
			subMeshBlueprints, // submesh blueprints,
			1, // active submesh count
			false // is static
		);

		if (!result) { return false; }

		// up the count
		mCount++;

		// reset basis vectors
		GenerateBasisVectorsFrom(
			0,
			0,
			0,
			basisVectors
		);
		// Terrain
		subMeshBlueprint0 = {};
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::TERRAIN;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::INVALID;

		result = UpdateBlueprint(
			1, // raw array index
			Vector3(0.f, 0.f, 0.f),
			Vector3(1.f),
			basisVectors,
			basisVectors.Forward, // surface normal
			EntityType::TERRAIN,
			EngineResources::MeshID::TERRAIN_0x0,
			subMeshBlueprints,
			1,
			true // is static
		);

		if (!result) { return false; }

		mCount++;

		// Wall
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::WALL;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::INVALID;
		result = UpdateBlueprint(
			2, // raw array index
			Vector3(0.f, 4.1f, -1.f), // center
			Vector3(1.5f, 2.f, .2f), // scale
			basisVectors,
			basisVectors.Forward, // surface normal
			EntityType::WALL,
			EngineResources::MeshID::CUBE,
			subMeshBlueprints,
			1,
			true // is static
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

		std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshBlueprints = {};

		// Player entity
		EntitySubMeshBlueprint& subMeshBlueprint0 = subMeshBlueprints[0];
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::PLAYER;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::WOOD_CRATE_01;

		EntitySubMeshBlueprint& subMeshBlueprint1 = subMeshBlueprints[1];
		subMeshBlueprint1 = {};
		subMeshBlueprint1.MaterialType = EngineResources::MaterialType::PLAYER;
		subMeshBlueprint1.TextureID = EngineResources::TextureID::WOOD_CRATE_02;

		// calculate the basis vectors
		BasisVectors basisVectors;
		GenerateBasisVectorsFrom(
			0,
			0,
			0,
			basisVectors
		);

		result = UpdateBlueprint(
			0, // raw array index
			Vector3(-10.f, 0.975f, -10.5f), // center
			Vector3(.75f, .75f, .75f), // scale
			basisVectors, // basis Vectors
			basisVectors.Forward, // surface normal
			EntityType::PLAYER, // entitytype
			EngineResources::MeshID::CUBE_TWO_SUBMESHES, // mesh id,
			subMeshBlueprints, // submesh blueprints,
			2, // active submesh count,
			false // isstatic 
		);

		if (!result) { return false; }

		// up the count
		mCount++;

		// reset the basis vectors
		GenerateBasisVectorsFrom(
			0,
			0,
			0,
			basisVectors
		);
		// Floor
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::WALL;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::CHECKBOARD;
		result = UpdateBlueprint(
			1, // raw array index
			Vector3(0.f),
			Vector3(32.f, 0.2f, 32.f),
			basisVectors,
			basisVectors.Forward, // surface normal
			EntityType::FLOOR,
			EngineResources::MeshID::CUBE,
			subMeshBlueprints,
			1,
			true // is static
		);

		if (!result) { return false; }

		mCount++;

		// Mirror
		GenerateBasisVectorsFrom(
			180,
			0,
			0,
			basisVectors
		);
		subMeshBlueprint0 = {};
		subMeshBlueprint0.MaterialType = EngineResources::MaterialType::WALL;
		subMeshBlueprint0.TextureID = EngineResources::TextureID::INVALID;
		subMeshBlueprint1 = {};
		subMeshBlueprint1.MaterialType = EngineResources::MaterialType::MIRROR;
		subMeshBlueprint1.TextureID = EngineResources::TextureID::CHECKBOARD;
		result = UpdateBlueprint(
			2, // raw array index
			Vector3(0.f, 2.205f, -1.f), // center
			Vector3(1.5f, 2.f, .2f), // scale
			basisVectors,
			basisVectors.Forward, // surface normal
			EntityType::MIRROR,
			EngineResources::MeshID::CUBE_TWO_SUBMESHES,
			subMeshBlueprints,
			2, // mirror has 2 submeshses
			// todo: find a better way to know these things,
			true // isstatic
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
		BasisVectors basisVectors,
		Vector3 surfaceNormal,
		EntityType entityType,
		EngineResources::MeshID meshID,
		std::array<EntitySubMeshBlueprint, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshBlueprints,
		uint8_t activeSubMeshCount,
		bool isStatic
	) {
		if (index >= EngineConfig::EngineConfig::MAX_ENTITIES) {
			Logger::ERR(L"Cannot load more Entities");
			return false;
		}

		EntityBlueprint& blueprintAtIndex = mBlueprintBackingMemory[index];
		blueprintAtIndex.Center = center;
		blueprintAtIndex.Scale = scale;
		blueprintAtIndex.BasisVectors = basisVectors;
		blueprintAtIndex.SurfaceNormal = surfaceNormal;
		blueprintAtIndex.EntityType = entityType;
		blueprintAtIndex.MeshID = meshID;
		blueprintAtIndex.EntitySubMeshBlueprints = subMeshBlueprints;
		blueprintAtIndex.ActiveSubMeshCount = activeSubMeshCount;
		blueprintAtIndex.IsStatic = isStatic;

		return true;
	}

	void SceneFactory::GenerateBasisVectorsFrom(
		// Angles are in DX12 LH CW: 0=+Z, 90=+X, 180=-Z, 270=-X
		float yawDegrees, 
		float pitchDegrees,
		float rollDegrees,
		BasisVectors& basisVectors
	) {
		const float yawRad = DirectX::XMConvertToRadians(yawDegrees);
		const float pitchRad = DirectX::XMConvertToRadians(pitchDegrees);
		const float rollRad = DirectX::XMConvertToRadians(rollDegrees);

		// Build individual axis rotations
		const DirectX::XMMATRIX yawMat = DirectX::XMMatrixRotationY(yawRad);
		const DirectX::XMMATRIX pitchMat = DirectX::XMMatrixRotationX(pitchRad);
		const DirectX::XMMATRIX rollMat = DirectX::XMMatrixRotationZ(rollRad);

		// YXZ intrinsic composition = Z * X * Y
		const DirectX::XMMATRIX rotMat = DirectX::XMMatrixMultiply(
			rollMat,
			DirectX::XMMatrixMultiply(pitchMat, yawMat)
		);

		// Row-major extraction: Row 0=Right (+X), Row 1=Up (+Y), Row 2=Forward (+Z)
		// At 0 degrees, rotMat is the Identity matrix, so:
		// Row 0 becomes (1, 0, 0), Row 1 becomes (0, 1, 0), Row 2 becomes (0, 0, 1)
		DirectX::XMStoreFloat3(&basisVectors.Right.AsXMFLOAT3(), rotMat.r[0]);
		DirectX::XMStoreFloat3(&basisVectors.Up.AsXMFLOAT3(), rotMat.r[1]);
		DirectX::XMStoreFloat3(&basisVectors.Forward.AsXMFLOAT3(), rotMat.r[2]);
	}

#pragma endregion
}