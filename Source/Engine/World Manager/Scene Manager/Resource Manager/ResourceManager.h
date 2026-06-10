#pragma once

#include <cstdint>
#include <array>
#include "Materials Manager/MaterialsManager.h"
#include <memory>
#include "../Entity/Mesh/Mesh.h"
#include "Terrain Manager/TerrainManager.h"

namespace Engine::EngineResources {

	class ResourceManager {
	public:
		ResourceManager(uint16_t chunkSize);
		~ResourceManager();

		uint32_t GetMaterialCount() const noexcept;
		const Mesh* GetMesh(MeshID id);
		const std::array<Mesh, (uint32_t)MeshID::Count>& GetMeshes() const;
		std::array<Material, (uint16_t)MaterialType::Count>& GetMaterials() noexcept;

	private:
		MaterialsManager mMaterialsManager;
		std::array<Mesh, (uint32_t)MeshID::Count> mMeshes;
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