#include "WorldManager.h"

#include "../Debug System/DebugSystem.h"
#include "Scene Manager/Entity/Entity.h"
#include "WorldDimensions.h"

namespace Engine::EngineWorld {

	WorldManager::WorldManager() :
		mSceneManager(SceneManager()),
		mPlayer(Player())
	{}

	WorldManager::~WorldManager() 
	{}

#pragma region Public

	bool WorldManager::Initialize() {
		// todo

		// todo: load player save data's player center and 
		// load the world from that center pos
		// for now it's 0, 0, 0
		Vector3 center;
		float halfWidth = WorldDimensions::World_Half_Size;

		if (!mSceneManager.Initialize(center, halfWidth)) { return false; }

		if (!LoadScene()) { return false; }

		return true;
	}

	void WorldManager::Update(
		const IInputSystem* const inputSystem,
		float deltaTime, 
		float animationSpeed, 
		const BasisVectors& cameraBasisVectors
	) {
		// first prepare for new frame
		PrepareForUpdate();

		// then update player
		mPlayer.Update(
			deltaTime,
			inputSystem,
			cameraBasisVectors
		);

		// get collision candidates for player
		// todo: generalize for all dynamic objects
		std::vector<const Entity*> collisionCandidates;
		const AABB potentialFootprint = mPlayer.CalculatePotentialFootprintAABB();
		mSceneManager.GetPotentialCollisionsWithAABB(
			potentialFootprint,
			collisionCandidates
		);

		// generate physics entites from collision result
		Engine::EnginePhysics::CollisionResult collisionResult;
		std::vector<EnginePhysics::PhysicsEntity> collisionCandidatesPhysicsEntites;
		collisionCandidatesPhysicsEntites.reserve(collisionCandidates.size());

		for (int i = 0; i < collisionCandidates.size(); ++i) {
			const Entity* entity = collisionCandidates[i];
			EnginePhysics::PhysicsEntity physicsEntity;
			physicsEntity.AABB = entity->GetAABB();
			physicsEntity.ID = entity->GetID();
			physicsEntity.IsTerrain = entity->GetIsTerrainOrFloor();
			collisionCandidatesPhysicsEntites.push_back(physicsEntity);
		}

		// get the Y for current terrain/floor
		Entity& playerEntity = mSceneManager.GetPlayerEntity();

		// each chunk should have a single entity that represents
		// the "terrain" or the "floor" even if the floor may have "holes"
		// in them
		// TODO:
		EnginePhysics::PhysicsBody& physicsBody = playerEntity.GetPhysicsBody();
		EngineWorld::EntityTransformData& transformData = playerEntity.GetTransformData();

		float playerX = transformData.Center.x;
		float playerZ = transformData.Center.z;
		float proposedY = mSceneManager.GetProposedYOfTerrainOrFloor(
			playerX,
			playerZ,
			deltaTime
		);

		// resolve collision
		mPhysicsSystem.ResolveEntityMovement(
			playerEntity,
			collisionCandidatesPhysicsEntites,
			proposedY,
			collisionResult
		);

		// todo: tell all entities to handle collision result
		// for now it's just the player
		mPlayer.PostPhysicsUpdate(collisionResult);

		// log player X, Z
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%.2f", transformData.Center.x);
		std::string posX(buffer);
		snprintf(buffer, sizeof(buffer), "%.2f", transformData.Center.z);
		std::string posZ(buffer);

		std::string playerXZ =
			"PlayerX:" + posX  +
			"/PlayerZ:" + posZ + "\n";
		Engine::DebugSystem::DebugSystem::GetInstance().LogText(playerXZ);

		// finally update scene manager
		mSceneManager.Update(inputSystem, deltaTime, animationSpeed);

		// log number of entities we're gonna draw
		std::string noOfEntities =
			"No. Of Entities: " +
			std::to_string(GetEntityCount()) + "\n";
		Engine::DebugSystem::DebugSystem::GetInstance().LogText(noOfEntities);
	}

	uint32_t WorldManager::GetEntityCount() const noexcept {
		return mSceneManager.GetEntityCount();
	}

	uint32_t WorldManager::GetMaterialCount() const noexcept {
		return mSceneManager.GetMaterialCount();
	}

	uint32_t WorldManager::GetConstantBufferDataByteSizeOfEachEntity() const {
		return mSceneManager.GetConstantBufferDataByteSizeOfEachEntity();
	}

	uint32_t WorldManager::GetConstantBufferDataByteSizeOfEachPerPassObject() const {
		return mSceneManager.GetConstantBufferDataByteSizeOfEachPerPassObject();
	}

	uint32_t WorldManager::GetConstantBufferDataByteSizeOFEntityPerSubMeshObject() const noexcept {
		return mSceneManager.GetConstantBufferDataByteSizeOFEntityPerSubMeshObject();
	}

	uint32_t WorldManager::GetConstantBufferDataByteSizeOfEachMaterialObject() const noexcept {
		return sizeof(EngineResources::MaterialData);
	}

	const Vector4& WorldManager::GetAmbientLight() const noexcept {
		return mSceneManager.GetAmbientLight();
	}

	void WorldManager::GetLightsData(LightsArray16& lights) const {
		mSceneManager.GetLightsData(lights);
	}

	std::array<Entity, EngineConfig::EngineConfig::MAX_ENTITIES>& WorldManager::GetEntities() {
		return mSceneManager.GetEntities();
	}

	EngineResources::MaterialArray& WorldManager::GetMaterials() noexcept {
		return mSceneManager.GetMaterials();
	}

	EngineResources::TextureArray& WorldManager::GetTextures() noexcept {
		return mSceneManager.GetTextures();
	}

	Vector3 WorldManager::GetPlayerCenter() {
		return mSceneManager.GetPlayerEntity().GetTransformData().Center;
	}

	const EngineResources::MeshArray& WorldManager::GetMeshesToLoad() const noexcept {
		return mSceneManager.GetMeshesToLoad();
	}

#pragma endregion

#pragma region Private
	
	bool WorldManager::LoadScene() {
		bool isLoaded = false;
		SceneBlueprint blueprint;

		// define what scene you want to load
		const Scene scene = Scene::SINGLE_MIRROR;

		// get scene factory to load the scene
		mSceneFactory.LoadScene(
			scene,
			blueprint,
			isLoaded
		);

		if (!isLoaded || blueprint.Scene != scene) { return false; }

		if (!mSceneManager.LoadScene(blueprint)) { return false; }

		mPlayer.SetEntity(mSceneManager.GetPlayerEntity());

		return true;
	}

	void WorldManager::PrepareForUpdate() {
		mSceneManager.PrepareForUpdate();
	}

#pragma endregion
}