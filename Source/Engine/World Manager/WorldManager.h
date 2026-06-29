#pragma once

#include <array>
#include <memory>
#include "Player/Player.h"
#include "../Physics System/PhysicsSystem.h"
#include "Scene/SceneFactory.h"
#include "Scene Manager/SceneManager.h"
#include "../Simulation/SimulationDataStructures.h"
#include <vector>

class Entity;
class IInputSystem;

namespace Engine::EngineWorld {

	class WorldManager {
	public:
		WorldManager();
		~WorldManager();

		WorldManager(const WorldManager& rhs) = delete;
		WorldManager& operator=(const WorldManager& rhs) = delete;
		WorldManager(WorldManager&&) = delete;
		WorldManager& operator=(WorldManager&&) = delete;

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
		uint32_t GetConstantBufferDataByteSizeOFEntityPerSubMeshObject() const noexcept;
		const Vector4& GetAmbientLight() const noexcept;

		void GetLightsData(LightsArray16& lights) const;

		std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& GetEntities();

		void GetEntitiesForReflectionPass(
			std::vector<const Entity*>& entities
		);

		EngineResources::MaterialArray& GetMaterials() noexcept;

		EngineResources::TextureArray& GetTextures() noexcept;

		Vector3 GetPlayerCenter();

		const EngineResources::MeshArray& GetMeshesToLoad() const noexcept;

		bool HasActiveMirrors() const noexcept;

		const EngineSimulation::MirrorPlaneQueryResult GetMirrorPlaneQueryResult() const noexcept;

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