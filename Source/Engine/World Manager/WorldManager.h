#pragma once

#include <array>
#include <memory>
#include "Player/Player.h"
#include "../Physics System/PhysicsSystem.h"
#include "Scene/SceneFactory.h"
#include "Scene Manager/SceneManager.h"
#include <vector>

class Entity;
class IInputSystem;

namespace Engine::EngineWorld {

	class WorldManager {
	public:
		WorldManager();
		~WorldManager();

		bool Initialize();

		void Update(
			const IInputSystem* const inputSystem, 
			float deltaTime, 
			float animationSpeed,
			const BasisVectors& cameraBasisVectors
		);

		uint32_t GetEntityCount() const noexcept;
		uint32_t GetMaterialCount() const noexcept;
		uint32_t GetConstantBufferDataByteSizeOfEachEntity() const;
		uint32_t GetConstantBufferDataByteSizeOfEachPerPassObject() const;
		uint32_t GetConstantBufferDataByteSizeOfEachMaterialObject() const noexcept;
		const Vector4& GetAmbientLight() const noexcept;
		void GetLightsData(LightsArray16& lights) const;
		std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& GetEntities();
		EngineResources::MaterialArray& GetMaterials() noexcept;
		EngineResources::TextureArray& GetTextures() noexcept;
		const Vector3& GetPlayerCenter() const;
		Entity& GetPlayerEntity();
		void GetMeshesToLoad(std::vector<const Mesh*>& meshes);

	private:
		SceneManager mSceneManager;
		SceneFactory mSceneFactory;
		Player mPlayer;
		EnginePhysics::PhysicsSystem mPhysicsSystem;

	private:
		bool LoadScene();
		void PrepareForUpdate();
	};
}