#pragma once

#include <array>
#include "Material/Material.h"

namespace Engine::EngineResources {

	using MaterialArray = std::array<Material, (uint16_t)MaterialType::COUNT>;

	class MaterialsManager {
	public:
		MaterialsManager();
		~MaterialsManager();

		bool Initialize();

		uint32_t GetMaterialCount() const noexcept;
		uint16_t GetSizeOfEachMaterialForCb() const noexcept;
		MaterialArray& GetMaterials() noexcept;

	private:
		MaterialArray mMaterials;

	private:
		void CreateMaterials();
		void CreatePlayerMaterial();
		void CreateWallMaterial();
		void CreateTerrainMaterial();
	};
}