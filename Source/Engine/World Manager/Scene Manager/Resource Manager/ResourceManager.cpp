#include "ResourceManager.h"

#include <DirectXColors.h>

namespace Engine::EngineResources {

	ResourceManager::ResourceManager(uint16_t chunkSize) :
		mLoadedBitMask(0),
		mTerrainManager(TerrainManager(chunkSize))
	{}

	ResourceManager::~ResourceManager() {}

	bool ResourceManager::Initialize() {
		if (!mMaterialsManager.Initialize()) { return false; }

		if (!mTextureManager.Initialize()) { return false; }

		return true;
	}

	uint32_t ResourceManager::GetMaterialCount() const noexcept {
		return mMaterialsManager.GetMaterialCount();
	}

	const Mesh* ResourceManager::GetMesh(MeshID id) {
		if (id >= MeshID::Count) {
			return nullptr;
		}

		size_t index = size_t(id);
		Mesh* mesh = nullptr;

		if (!GetIsLoaded(index)) {
			mesh = &mMeshes[index];

			switch (id) {
			case MeshID::Cube:
				CreateCubeMesh(mesh);
				break;

			case MeshID::Terrain0x0:
				CreateTerrian0x0(mesh);
				break;
			}

			SetIsLoaded(index);
		}
		else {
			mesh = &mMeshes[index];
		}

		return mesh;
	}

	const MeshArray& ResourceManager::GetMeshes() const  {
		return mMeshes;
	}

	MaterialArray& ResourceManager::GetMaterials() noexcept {
		return mMaterialsManager.GetMaterials();
	}

	TextureArray& ResourceManager::GetTextures() noexcept {
		return mTextureManager.GetTextures();
	}

#pragma region Private

	void ResourceManager::CreateCubeMesh(Mesh* mesh) {
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;

		// Define the 6 unit directions for a cube's faces based on Left-Handed (+Z into screen)
		Vector3 normals[6] = {
			Vector3(0.0f,  0.0f, -1.0f), // Front (pointing out of screen toward eye)
			Vector3(0.0f,  0.0f,  1.0f), // Back  (pointing into screen away from eye)
			Vector3(1.0f,  0.0f,  0.0f), // Right
			Vector3(-1.0f,  0.0f,  0.0f), // Left
			Vector3(0.0f,  1.0f,  0.0f), // Top
			Vector3(0.0f, -1.0f,  0.0f)  // Bottom
		};

		// 1. Front Face (Z = -1, closest to camera. Looking straight at it, CW is TL -> TR -> BR -> BL)
		vertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[0], Vector2(0.0f, 0.0f) }); // 0: Top-Left
		vertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[0], Vector2(1.0f, 0.0f) }); // 1: Top-Right
		vertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[0], Vector2(1.0f, 1.0f) }); // 2: Bottom-Right
		vertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[0], Vector2(0.0f, 1.0f) }); // 3: Bottom-Left

		// 2. Back Face (Z = 1, furthest away. Looking from behind the cube, CW is TL -> TR -> BR -> BL)
		vertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[1], Vector2(0.0f, 0.0f) }); // 4: Top-Left (from back view)
		vertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[1], Vector2(1.0f, 0.0f) }); // 5: Top-Right (from back view)
		vertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[1], Vector2(1.0f, 1.0f) }); // 6: Bottom-Right (from back view)
		vertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[1], Vector2(0.0f, 1.0f) }); // 7: Bottom-Left (from back view)

		// 3. Right Face (X = 1. Looking straight at it, CW is TL -> TR -> BR -> BL)
		vertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[2], Vector2(0.0f, 0.0f) }); // 8: Top-Left
		vertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[2], Vector2(1.0f, 0.0f) }); // 9: Top-Right
		vertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[2], Vector2(1.0f, 1.0f) }); // 10: Bottom-Right
		vertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[2], Vector2(0.0f, 1.0f) }); // 11: Bottom-Left

		// 4. Left Face (X = -1. Looking straight at it, CW is TL -> TR -> BR -> BL)
		vertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[3], Vector2(0.0f, 0.0f) }); // 12: Top-Left
		vertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[3], Vector2(1.0f, 0.0f) }); // 13: Top-Right
		vertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[3], Vector2(1.0f, 1.0f) }); // 14: Bottom-Right
		vertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[3], Vector2(0.0f, 1.0f) }); // 15: Bottom-Left

		// 5. Top Face (Y = 1. Looking down at it from above, CW is TL -> TR -> BR -> BL)
		vertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[4], Vector2(0.0f, 0.0f) }); // 16: Top-Left
		vertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[4], Vector2(1.0f, 0.0f) }); // 17: Top-Right
		vertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[4], Vector2(1.0f, 1.0f) }); // 18: Bottom-Right
		vertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[4], Vector2(0.0f, 1.0f) }); // 19: Bottom-Left

		// 6. Bottom Face (Y = -1. Looking up at it from below, CW is TL -> TR -> BR -> BL)
		vertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[5], Vector2(0.0f, 0.0f) }); // 20: Top-Left
		vertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[5], Vector2(1.0f, 0.0f) }); // 21: Top-Right
		vertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[5], Vector2(1.0f, 1.0f) }); // 22: Bottom-Right
		vertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[5], Vector2(0.0f, 1.0f) }); // 23: Bottom-Left

		// Core Index Winding Loop
		for (uint16_t i = 0; i < 6; ++i) {
			uint16_t baseVertex = i * 4;

			// Triangle 1: Top-Left -> Top-Right -> Bottom-Right (Clockwise)
			indices.push_back(baseVertex + 0);
			indices.push_back(baseVertex + 1);
			indices.push_back(baseVertex + 2);

			// Triangle 2: Top-Left -> Bottom-Right -> Bottom-Left (Clockwise)
			indices.push_back(baseVertex + 0);
			indices.push_back(baseVertex + 2);
			indices.push_back(baseVertex + 3);
		}

		mesh->Load(MeshID::Cube, vertices, indices);
	}

	void ResourceManager::CreateTerrian0x0(Mesh* mesh) {
		std::vector<Engine::Vertex> vertices;

		const uint16_t vertexCount = mTerrainManager.TotalNumberOfVerticesForChunk(TerrainLOD::HIGH);
		vertices.reserve(vertexCount);

		std::vector<uint16_t> indices;
		const uint32_t indexCount = mTerrainManager.TotalNumberOfIndicesForChunk(TerrainLOD::HIGH);
		indices.reserve(indexCount);

		mTerrainManager.GenerateTerrainFor(
			0,
			0,
			TerrainLOD::HIGH,
			vertices,
			indices
		);
		mesh->Load(MeshID::Terrain0x0, vertices, indices);
	}

	bool ResourceManager::GetIsLoaded(size_t index) {
		assert(index >= 0 && index < 64);

		uint64_t mask = 1ULL << index;
		return (mLoadedBitMask & mask) != 0;
	}

	void ResourceManager::SetIsLoaded(size_t index) {
		assert(index >= 0 && index < 64);

		uint64_t mask = 1ULL << index;
		mLoadedBitMask |= mask;
	}

	void ResourceManager::SetIsUnloaded(size_t index) {
		assert(index >= 0 && index < 64);

		uint64_t mask = 1ULL << index;
		uint64_t maskNegate = ~mask;
		mLoadedBitMask &= maskNegate;
	}

#pragma endregion
}