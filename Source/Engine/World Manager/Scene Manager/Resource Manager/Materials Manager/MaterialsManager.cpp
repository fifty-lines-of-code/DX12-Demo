#include "MaterialsManager.h"

namespace Engine::EngineResources {
	MaterialsManager::MaterialsManager() {}

	MaterialsManager::~MaterialsManager() {}

	void MaterialsManager::Initialize() {
		CreateMaterials();
	}

	uint32_t MaterialsManager::GetMaterialCount() const noexcept {
		return (uint32_t)MaterialType::Count;
	}

	uint16_t MaterialsManager::GetSizeOfEachMaterialForCb() const noexcept { 
		return sizeof(MaterialData);
	}

	std::array<Material, (uint16_t)MaterialType::Count>& MaterialsManager::GetMaterials() noexcept {
		return mMaterials;
	}

#pragma region Private

	void MaterialsManager::CreateMaterials() {
		CreatePlayerMaterial();
		CreateWallMaterial();
		CreateTerrainMaterial();
	}

	void MaterialsManager::CreatePlayerMaterial() {
		MaterialType type = MaterialType::Player;
		Material& playerMaterial = mMaterials[(uint16_t)type];
		MaterialData playerMaterialData;

		// 1. Pure White Diffuse at maximum intensity (RGBA)
		playerMaterialData.DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		// 2. High Fresnel reflectivity for that bright, polished shell highlight
		playerMaterialData.FresnelR0 = Vector3(0.95f, 0.95f, 0.95f);

		// 3. Ultra-low roughness (0.01f to 0.05f) 
		// This concentrates incoming light into a super tight, blindingly sharp specular highlight, 
		// making the cube look slick, glassy, and intensely energetic.
		playerMaterialData.Roughness = 0.02f;

		playerMaterial.SetType(type);
		playerMaterial.SetData(playerMaterialData);
		playerMaterial.SetIsDirty(true);
	}

	void MaterialsManager::CreateWallMaterial() {
		MaterialType type = MaterialType::Wall;
		Material& wallMaterial = mMaterials[(uint16_t)type];
		MaterialData wallMaterialData;

		// 1. Medium-Dark Slate Gray (RGBA)
		// A concrete or dark stone wall should absorb a decent amount of light.
		wallMaterialData.DiffuseAlbedo = Vector4(0.35f, 0.35f, 0.35f, 1.0f);

		// 2. Low Fresnel reflectivity (Dielectric / Non-Metal)
		// Real-world stones and bricks reflect very little light when viewed head-on.
		// 0.02f to 0.05f is the standard physical value for non-metals.
		wallMaterialData.FresnelR0 = Vector3(0.04f, 0.04f, 0.04f);

		// 3. High Roughness (0.7f to 0.9f)
		// This scatters incoming light in all directions, completely blurring out 
		// any sharp specular highlights. It gives the cube a flat, matte, textured feel.
		wallMaterialData.Roughness = 0.8f;

		wallMaterial.SetType(type);
		wallMaterial.SetData(wallMaterialData);
		wallMaterial.SetIsDirty(true);
	}

	void MaterialsManager::CreateTerrainMaterial() {
		MaterialType type = MaterialType::Terrain;
		Material& terrianMaterial = mMaterials[(uint16_t)type];
		MaterialData terrainMaterialData;

		// 1. Organic Forest Green / Earthy Khaki (RGBA)
		// A desaturated, natural green that absorbs light like foliage and rich soil.
		terrainMaterialData.DiffuseAlbedo = Vector4(0.22f, 0.38f, 0.24f, 1.0f);

		// 2. Very Low Fresnel reflectivity (0.02f to 0.03f)
		// Soil, rock, and vegetation have almost no specular reflection when viewed head-on.
		// Keeping this near rock-bottom ensures it behaves like a true organic dielectric.
		terrainMaterialData.FresnelR0 = Vector3(0.02f, 0.02f, 0.02f);

		// 3. Peak Roughness (0.85f to 0.95f)
		// Mountains are highly micro-textured and irregular. This completely obliterates 
		// any pinpoint light reflections, spreading incoming sunlight into a soft, 
		// natural outdoor gradient across your heightmap or terrain mesh.
		terrainMaterialData.Roughness = 0.90f;

		terrianMaterial.SetType(type);
		terrianMaterial.SetData(terrainMaterialData);
		terrianMaterial.SetIsDirty(true);
	}

#pragma endregion
}