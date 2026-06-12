#pragma once

#include <cstdint>
#include <array>
#include "Materials Manager/MaterialsManager.h"
#include <memory>
#include "../Entity/Mesh/Mesh.h"
#include "Terrain Manager/TerrainManager.h"
#include "Texture Manager/TextureManager.h"

namespace Engine::EngineResources {

	using MeshArray = std::array<Mesh, (uint32_t)MeshID::Count>;

	class ResourceManager {
	public:
		ResourceManager(uint16_t chunkSize);
		~ResourceManager();

		bool Initialize();
		uint32_t GetMaterialCount() const noexcept;
		const Mesh* GetMesh(MeshID id);
		const MeshArray& GetMeshes() const;
		MaterialArray& GetMaterials() noexcept;
		TextureArray& GetTextures() noexcept;

	private:
		MaterialsManager mMaterialsManager;
		MeshArray mMeshes;
		TextureManager mTextureManager;
		// assuming we will have 64 unique Meshes
		// if we have more than that, time to create an array
		// and index into the uint64 inside the array based on div by 64 and modulo 64
		uint64_t mLoadedBitMask;
		TerrainManager mTerrainManager;

	private:
		static void CreateCubeMesh(Mesh* mesh);
		void CreateTerrian0x0(Mesh* mesh);
		bool GetIsLoaded(size_t index);
		void SetIsLoaded(size_t index);
		void SetIsUnloaded(size_t index);
	};
}