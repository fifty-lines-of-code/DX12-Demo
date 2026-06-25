#pragma once

#include <array>
#include "Chunks Manager/ChunksManager.h"
#include "../../EngineConfig.h"
#include "Entity/Entity.h"
#include "Lights/Lights Manager/LightsManager.h"
#include <memory>
#include "../Scene Manager/Resource Manager/Mesh Generator/Mesh/Mesh.h"
#include "OctTree/OctTree.h"
#include "Resource Manager/ResourceManager.h"
#include "../Scene/SceneBlueprint.h"
#include <unordered_map>

class Camera;
class IInputSystem;

namespace Engine::EngineWorld {

	class SceneManager {
	public:
		SceneManager();
		~SceneManager();

		bool Initialize(const Vector3& center, float halfWidth);
		bool LoadScene(SceneBlueprint& sceneBlueprint);
		void GetPotentialCollisionsWithAABB(
			const AABB& playerPotentialAABB,
			std::vector<const Entity*>& candidates
		);

		void Update(
			const IInputSystem* const inputSystem, 
			float deltaTime,
			float animationSpeed
		);

		uint32_t GetEntityCount() const noexcept;
		uint32_t GetMaterialCount() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachEntity() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachMaterialObject() const noexcept;
		const Vector4& GetAmbientLight() const noexcept;
		void GetLightsData(LightsArray16& lights) const;
		const EngineResources::MeshArray& GetMeshesToLoad() const noexcept;
		Entity& GetPlayerEntity();

		std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& GetEntities();	
		EngineResources::MaterialArray& GetMaterials() noexcept;
		EngineResources::TextureArray& GetTextures() noexcept;

		void PrepareForUpdate();

		float GetProposedYOfTerrainOrFloor(
			float entityX, 
			float entityZ,
			float deltaTime
		);

	private:
		static constexpr uint32_t PLAYER_INDEX = 0;
		uint32_t mEntityCount;
		OctTree mOctTree;
		std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES> mEntities;
		EngineResources::ResourceManager mResourceManager;
		ChunksManager mChunksManager;
		LightsManager mLightsManager;
		std::vector<uint32_t> mIndexesOfDynamicEntities;

	private:
		bool GeneratePlayerEntity(SceneBlueprint& sceneblueprint);
		void PrepareForCollisionPass();
		float CalculateProposedYOfTerrain(
			const EngineResources::Mesh& terrainMesh,
			float entityX,
			float enityZ,
			const Vector2& chunkCenterXZ,
			float deltaTime
		);
	};
}