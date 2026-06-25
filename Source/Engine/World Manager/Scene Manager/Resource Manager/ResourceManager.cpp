#include "ResourceManager.h"

#include "../../../../Helper/Helper.h"

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

	Mesh* ResourceManager::GetMesh(MeshID id) {
		if (id >= MeshID::COUNT) {
			return nullptr;
		}

		size_t index = size_t(id);
		Mesh* mesh = nullptr;

		if (!GetIsLoaded(index)) {
			mesh = &mMeshes[index];

			switch (id) {
			case MeshID::CUBE:
				CreateCubeMesh(mesh);
				break;

			case MeshID::TERRAIN_0x0:
				CreateTerrian(
					mesh,
					0.f, 
					0.f,
					id);
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

	TerrainLOD ResourceManager::GetTerrainLOD() const noexcept { 
		return mCurrentTerrainLOD; 
	}

#pragma region Private

	void ResourceManager::CreateCubeMesh(Mesh* mesh) const noexcept {
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
		std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshes;

		mMeshGenerator.GenerateCubeMesh(
			vertices, 
			indices, 
			subMeshes
		);

		mesh->Load(
			MeshID::CUBE,
			vertices,
			indices,
			subMeshes,
			1 // only 1 active submesh
		);
	}

	void ResourceManager::CreateTerrian(
		Mesh* mesh,
		float centerX,
		float centerZ,
		MeshID meshID
	) {
		std::vector<Engine::Vertex> vertices;

		const uint16_t vertexCount = mTerrainManager.TotalNumberOfVerticesForChunk(mCurrentTerrainLOD);
		vertices.reserve(vertexCount);

		std::vector<uint16_t> indices;
		const uint32_t indexCount = mTerrainManager.TotalNumberOfIndicesForChunk(
			mCurrentTerrainLOD
		);
		indices.reserve(indexCount);

		mTerrainManager.GenerateTerrainFor(
			centerX,
			centerZ,
			mCurrentTerrainLOD,
			vertices,
			indices
		);

		// set submesh data
		std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshes;
		SubMesh& subMesh0 = subMeshes[0];

		subMesh0.IndexCount = (uint32_t)indexCount;
		subMesh0.StartIndexLocation = 0;
		subMesh0.BaseVertexLocation = 0;

		mesh->Load(
			meshID, 
			vertices, 
			indices, 
			subMeshes,
			1 // only 1 active submesh
		);
	}

	bool ResourceManager::GetIsLoaded(size_t index) {
		ENGINE_ASSERT((index >= 0 && index < 64), "Index is out of bounds!");

		uint64_t mask = 1ULL << index;
		return (mLoadedBitMask & mask) != 0;
	}

	void ResourceManager::SetIsLoaded(size_t index) {
		ENGINE_ASSERT(index >= 0 && index < 64, "Index is out of bounds!");

		uint64_t mask = 1ULL << index;
		mLoadedBitMask |= mask;
	}

	void ResourceManager::SetIsUnloaded(size_t index) {
		ENGINE_ASSERT(index >= 0 && index < 64, "Index is out of bounds!");

		uint64_t mask = 1ULL << index;
		uint64_t maskNegate = ~mask;
		mLoadedBitMask &= maskNegate;
	}

#pragma endregion
}