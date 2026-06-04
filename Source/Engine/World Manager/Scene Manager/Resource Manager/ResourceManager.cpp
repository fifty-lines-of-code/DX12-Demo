#include "ResourceManager.h"

#include <DirectXColors.h>

namespace Engine {

	ResourceManager::ResourceManager(uint16_t chunkSize) :
		mLoadedBitMask(0),
		mTerrainManager(TerrainManager(chunkSize))
	{}

	ResourceManager::~ResourceManager() {}

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

#pragma region Private

	void ResourceManager::CreateCubeMesh(Mesh* mesh) {
		std::vector<Vertex> vertices = {
			Vertex({ Vector3(-.5f, -.5f, -.5f), Vector4(DirectX::Colors::White.f) }),
			Vertex({ Vector3(-.5f, +.5f, -.5f), Vector4(DirectX::Colors::Black.f) }),
			Vertex({ Vector3(+.5f, +.5f, -.5f), Vector4(DirectX::Colors::Red.f) }),
			Vertex({ Vector3(+.5f, -.5f, -.5f), Vector4(DirectX::Colors::Green.f) }),
			Vertex({ Vector3(-.5f, -.5f, +.5f), Vector4(DirectX::Colors::Blue.f) }),
			Vertex({ Vector3(-.5f, +.5f, +.5f), Vector4(DirectX::Colors::Yellow.f) }),
			Vertex({ Vector3(+.5f, +.5f, +.5f), Vector4(DirectX::Colors::Cyan.f) }),
			Vertex({ Vector3(+.5f, -.5f, +.5f), Vector4(DirectX::Colors::Magenta.f) })
		};

		std::vector<uint16_t> indices = {
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7
		};
		mesh->Load(MeshID::Cube, vertices, indices);
	}

	void ResourceManager::CreateTerrian0x0(Mesh* mesh) {
		std::vector<Engine::Vertex> vertices;

		const uint16_t vertexCount = mTerrainManager.TotalNumberOfVerticesForChunk();
		vertices.reserve(vertexCount);

		std::vector<uint16_t> indices;

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