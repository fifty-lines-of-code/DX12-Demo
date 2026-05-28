#pragma once

#include <array>
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
		void GetCollisionsWithPlayer(
			const AABB& playerPotentialAABB,
			std::vector<const Entity*>& candidates
		);

		void Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed);

		uint32_t GetEntityCount() const;
		uint32_t GetConstantBufferDataByteSizeOfEachEntity() const;
		uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const;

		std::vector<const Mesh*> GetMeshesToLoad();
		Entity& GetPlayerEntity();

		static constexpr uint32_t MAX_ENTITIES = 3;
		std::array<Entity, MAX_ENTITIES>& GetEntities();

		void PrepareForNewFrame();

	private:
		static constexpr uint8_t PLAYER_INDEX = 0;
		std::array<Entity, MAX_ENTITIES> mEntities;
		std::vector<uint32_t> mIndexesOfDynamicEntities;
		uint32_t mIDOfNextEntityThatWillBeCreated = 0;
		ResourceManager mResourceManager;
		std::unordered_map<MeshID, const Mesh*> mMeshesToLoad;
		OctTree mOctTree;

	private:
		bool GeneratePlayerEntity();
		bool GenerateBasicScene();
		void PrepareForCollisionPass();
	};
}