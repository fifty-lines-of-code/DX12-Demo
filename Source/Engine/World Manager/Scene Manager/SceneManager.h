#pragma once

#include <array>
#include "Chunks Manager/ChunksManager.h"
#include "Entity/Entity.h"
#include "Lights/Lights Manager/LightsManager.h"
#include <memory>
#include "../Scene Manager/Entity/Mesh/Mesh.h"
#include "OctTree/OctTree.h"
#include "Resource Manager/ResourceManager.h"
#include <unordered_map>

class Camera;
class IInputSystem;

namespace Engine {

	class SceneManager {
	public:
		SceneManager();
		~SceneManager();

		bool Initialize(const Vector3& center, float halfWidth);
		bool LoadScene();
		void GetPotentialCollisionsWithAABB(
			const AABB& playerPotentialAABB,
			std::vector<const Entity*>& candidates
		);

		void Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed);

		uint32_t GetEntityCount() const noexcept;
		uint32_t GetMaterialCount() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachEntity() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachMaterialObject() const noexcept;
		const Vector4& GetAmbientLight() const noexcept;
		void GetLightsData(LightsArray16& lights) const;
		void GetMeshesToLoad(std::vector<const Mesh*>& meshes);
		Entity& GetPlayerEntity();

		static constexpr uint32_t MAX_ENTITIES = 3;
		std::array<Entity, MAX_ENTITIES>& GetEntities();	
		std::array<EngineResources::Material, (uint16_t)EngineResources::MaterialType::Count>& GetMaterials() noexcept;

		void PrepareForUpdate();

	private:
		static constexpr uint8_t PLAYER_INDEX = 0;

		OctTree mOctTree;
		std::array<Entity, MAX_ENTITIES> mEntities;
		EngineResources::ResourceManager mResourceManager;
		ChunksManager mChunksManager;
		LightsManager mLightsManager;
		std::vector<uint32_t> mIndexesOfDynamicEntities;

	private:
		bool GeneratePlayerEntity();
		void PrepareForCollisionPass();
	};
}