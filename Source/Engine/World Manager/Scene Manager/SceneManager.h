#pragma once

#include <array>
#include "Chunks Manager/ChunksManager.h"
#include "Entity/Entity.h"
#include <memory>
#include "../Scene Manager/Entity/Mesh/Mesh.h"
#include "OctTree/OctTree.h"
#include "Resource Manager/ResourceManager.h"
#include <unordered_map>
#include <vector>

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

		uint32_t GetEntityCount() const;
		uint32_t GetConstantBufferDataByteSizeOfEachEntity() const;
		uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const;

		void GetMeshesToLoad(std::vector<const Mesh*>& meshes);
		Entity& GetPlayerEntity();

		static constexpr uint32_t MAX_ENTITIES = 3;
		std::array<Entity, MAX_ENTITIES>& GetEntities();

		void PrepareForUpdate();

	private:
		static constexpr uint8_t PLAYER_INDEX = 0;

		OctTree mOctTree;
		std::array<Entity, MAX_ENTITIES> mEntities;
		ChunksManager mChunksManager;
		std::unordered_map<MeshID, const Mesh*> mMeshesToLoad;
		ResourceManager mResourceManager;
		std::vector<uint32_t> mIndexesOfDynamicEntities;

	private:
		bool GeneratePlayerEntity();
		void PrepareForCollisionPass();
	};
}