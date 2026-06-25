#pragma once

#include <array>
#include <cstdint>
#include "Materials Manager/MaterialsManager.h"
#include <memory>
#include "Mesh Generator/MeshGenerator.h"
#include "Terrain Manager/TerrainManager.h"
#include "Texture Manager/TextureManager.h"

namespace Engine::EngineResources {

	using MeshArray = std::array<Mesh, (uint32_t)MeshID::COUNT>;

	class ResourceManager {
	public:
		ResourceManager(uint16_t chunkSize);
		~ResourceManager();

		bool Initialize();
		uint32_t GetMaterialCount() const noexcept;
		Mesh* GetMesh(MeshID id);
		const MeshArray& GetMeshes() const;
		MaterialArray& GetMaterials() noexcept;
		TextureArray& GetTextures() noexcept;
		TerrainLOD GetTerrainLOD() const noexcept;

	private:
		TextureManager mTextureManager;
		MeshArray mMeshes;
		MaterialsManager mMaterialsManager;
		// assuming we will have 64 unique Meshes
		// if we have more than that, time to create an array
		// and index into the uint64 inside the array based on div by 64 and modulo 64
		uint64_t mLoadedBitMask;
		TerrainManager mTerrainManager;
		MeshGenerator mMeshGenerator;
		TerrainLOD mCurrentTerrainLOD = TerrainLOD::HIGH;

	private:
		void CreateCubeMesh(Mesh& mesh) const noexcept;
		void CreateMirrorMesh(Mesh& mesh) const noexcept;
		void CreateTerrian(
			Mesh& mesh,
			float centerX,
			float centerZ,
			MeshID meshID
		);
		bool GetIsLoaded(size_t index);
		void SetIsLoaded(size_t index);
		void SetIsUnloaded(size_t index);
	};
}