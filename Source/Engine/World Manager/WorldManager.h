#pragma once

#include <array>
#include <memory>
#include "Player/Player.h"
#include "Scene Manager/SceneManager.h"
#include <vector>

class Entity;
class IInputSystem;

namespace Engine {

	class WorldManager {
	public:
		WorldManager();
		~WorldManager();

		bool Initialize();

		void Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed, const BasisVectors& cameraBasisVectors);

		uint32_t GetEntityCount() const;
		uint32_t GetConstantBufferDataByteSizeOfEachEntity() const;
		uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const;
		std::array<Entity, SceneManager::MAX_ENTITIES>& GetEntities();
		const Vector3& GetPlayerCenter() const;
		Entity& GetPlayerEntity();
		std::vector<const Mesh*> GetMeshesToLoad();

	private:
		SceneManager mSceneManager;
		Player mPlayer;

	private:
		bool LoadScene();
		void PrepareForNewFrame();
	};
}