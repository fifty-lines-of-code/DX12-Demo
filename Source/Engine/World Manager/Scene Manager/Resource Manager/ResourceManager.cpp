#include "ResourceManager.h"

#include <DirectXColors.h>

namespace Engine::EngineResources {

	ResourceManager::ResourceManager(uint16_t chunkSize) :
		mLoadedBitMask(0),
		mTerrainManager(TerrainManager(chunkSize))
	{
		mMaterialsManager.Initialize();
	}

	ResourceManager::~ResourceManager() {}

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

	const std::array<Mesh, (uint32_t)MeshID::Count>& ResourceManager::GetMeshes() const  {
		return mMeshes;
	}

	std::array<Material, (uint16_t)MaterialType::Count>& ResourceManager::GetMaterials() noexcept {
		return mMaterialsManager.GetMaterials();
	}

#pragma region Private

	void ResourceManager::CreateCubeMesh(Mesh* mesh) {
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;

		// Define the 6 unit directions for a cube's faces based on +Z into screen
		Engine::Vector3 normals[6] = {
			Engine::Vector3(0.0f,  0.0f, -1.0f), // Front (pointing out of screen toward eye)
			Engine::Vector3(0.0f,  0.0f,  1.0f), // Back  (pointing into screen away from eye)
			Engine::Vector3(1.0f,  0.0f,  0.0f), // Right
			Engine::Vector3(-1.0f,  0.0f,  0.0f), // Left
			Engine::Vector3(0.0f,  1.0f,  0.0f), // Top
			Engine::Vector3(0.0f, -1.0f,  0.0f)  // Bottom
		};

		// 1. Front Face (Z = -1, closest to camera. Looking at it, CW order is TL -> TR -> BR -> BL)
		vertices.push_back({ Engine::Vector3(-1.0f,  1.0f, -1.0f), normals[0] }); // 0: Top-Left
		vertices.push_back({ Engine::Vector3(1.0f,  1.0f, -1.0f), normals[0] }); // 1: Top-Right
		vertices.push_back({ Engine::Vector3(1.0f, -1.0f, -1.0f), normals[0] }); // 2: Bottom-Right
		vertices.push_back({ Engine::Vector3(-1.0f, -1.0f, -1.0f), normals[0] }); // 3: Bottom-Left

		// 2. Back Face (Z = 1, furthest away. Looking through the cube, CW order is TL -> TR -> BR -> BL)
		vertices.push_back({ Engine::Vector3(1.0f,  1.0f,  1.0f), normals[1] }); // 4: Top-Left from back view
		vertices.push_back({ Engine::Vector3(-1.0f,  1.0f,  1.0f), normals[1] }); // 5: Top-Right from back view
		vertices.push_back({ Engine::Vector3(-1.0f, -1.0f,  1.0f), normals[1] }); // 6: Bottom-Right from back view
		vertices.push_back({ Engine::Vector3(1.0f, -1.0f,  1.0f), normals[1] }); // 7: Bottom-Left from back view

		// 3. Right Face (X = 1. Looking at it, CW order is TL -> TR -> BR -> BL)
		vertices.push_back({ Engine::Vector3(1.0f,  1.0f, -1.0f), normals[2] }); // 8: Top-Left
		vertices.push_back({ Engine::Vector3(1.0f,  1.0f,  1.0f), normals[2] }); // 9: Top-Right
		vertices.push_back({ Engine::Vector3(1.0f, -1.0f,  1.0f), normals[2] }); // 10: Bottom-Right
		vertices.push_back({ Engine::Vector3(1.0f, -1.0f, -1.0f), normals[2] }); // 11: Bottom-Left

		// 4. Left Face (X = -1. Looking at it, CW order is TL -> TR -> BR -> BL)
		vertices.push_back({ Engine::Vector3(-1.0f,  1.0f,  1.0f), normals[3] }); // 12: Top-Left
		vertices.push_back({ Engine::Vector3(-1.0f,  1.0f, -1.0f), normals[3] }); // 13: Top-Right
		vertices.push_back({ Engine::Vector3(-1.0f, -1.0f, -1.0f), normals[3] }); // 14: Bottom-Right
		vertices.push_back({ Engine::Vector3(-1.0f, -1.0f,  1.0f), normals[3] }); // 15: Bottom-Left

		// 5. Top Face (Y = 1. Looking down at it, CW order is TL -> TR -> BR -> BL)
		vertices.push_back({ Engine::Vector3(-1.0f,  1.0f,  1.0f), normals[4] }); // 16: Top-Left
		vertices.push_back({ Engine::Vector3(1.0f,  1.0f,  1.0f), normals[4] }); // 17: Top-Right
		vertices.push_back({ Engine::Vector3(1.0f,  1.0f, -1.0f), normals[4] }); // 18: Bottom-Right
		vertices.push_back({ Engine::Vector3(-1.0f,  1.0f, -1.0f), normals[4] }); // 19: Bottom-Left

		// 6. Bottom Face (Y = -1. Looking up at it, CW order is TL -> TR -> BR -> BL)
		vertices.push_back({ Engine::Vector3(-1.0f, -1.0f, -1.0f), normals[5] }); // 20: Top-Left
		vertices.push_back({ Engine::Vector3(1.0f, -1.0f, -1.0f), normals[5] }); // 21: Top-Right
		vertices.push_back({ Engine::Vector3(1.0f, -1.0f,  1.0f), normals[5] }); // 22: Bottom-Right
		vertices.push_back({ Engine::Vector3(-1.0f, -1.0f,  1.0f), normals[5] }); // 23: Bottom-Left

		// using the Top-Left (0), Top-Right (1), Bottom-Right (2), Bottom-Left (3) sequence.
		for (uint16_t i = 0; i < 6; ++i) {
			uint16_t baseVertex = i * 4;

			// Triangle 1: Top-Left -> Top-Right -> Bottom-Right
			indices.push_back(baseVertex + 0);
			indices.push_back(baseVertex + 1);
			indices.push_back(baseVertex + 2);

			// Triangle 2: Top-Left -> Bottom-Right -> Bottom-Left
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