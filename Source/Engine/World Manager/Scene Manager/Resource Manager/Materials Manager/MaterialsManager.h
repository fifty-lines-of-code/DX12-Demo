#pragma once

#include <array>
#include "Material/Material.h"

namespace Engine::EngineResources {

	class MaterialsManager {
	public:
		MaterialsManager();
		~MaterialsManager();

		void Initialize();


		uint32_t GetMaterialCount() const noexcept;
		uint16_t GetSizeOfEachMaterialForCb() const noexcept;
		std::array<Material, (uint16_t)MaterialType::Count>& GetMaterials() noexcept;

	private:
		std::array<Material, (uint16_t)MaterialType::Count> mMaterials;

	private:
		void CreateMaterials();
		void CreatePlayerMaterial();
		void CreateWallMaterial();
		void CreateTerrainMaterial();
	};
}