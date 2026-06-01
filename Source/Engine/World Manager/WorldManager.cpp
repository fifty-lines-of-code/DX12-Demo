#include "WorldManager.h"

#include "Scene Manager/Entity/Entity.h"
#include "WorldDimensions.h"

namespace Engine {

	WorldManager::WorldManager() :
		mSceneManager(SceneManager()),
		mPlayer(Player())
	{}

	WorldManager::~WorldManager() 
	{}

#pragma region Public

	bool WorldManager::Initialize() {
		// todo

		Vector3 center;
		float halfWidth = WorldDimensions::World_Half_Size;

		if (!mSceneManager.Initialize(center, halfWidth)) { return false; }

		if (!LoadScene()) { return false; }

		return true;
	}

	void WorldManager::Update(const IInputSystem* const inputSystem, float deltaTime, float animationSpeed, const BasisVectors& cameraBasisVectors) {
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

		Engine::EnginePhysics::CollisionResult collisionResult;
		mPhysicsSystem.ResolveEntityMovement(
			mSceneManager.GetPlayerEntity(),
			collisionCandidates,
			collisionResult
		);

		// todo: tell all entities to handle collision result
		// for now it's just the player
		mPlayer.PostPhysicsUpdate(collisionResult);

		// finally update scene manager
		mSceneManager.Update(inputSystem, deltaTime, animationSpeed);
	}

	uint32_t WorldManager::GetEntityCount() const {
		return mSceneManager.GetEntityCount();
	}

	uint32_t WorldManager::GetConstantBufferDataByteSizeOfEachEntity() const {
		return mSceneManager.GetConstantBufferDataByteSizeOfEachEntity();
	}

	uint32_t WorldManager::GetConstantBufferDataByteSizeOfEachPerPassObject() const {
		return mSceneManager.GetConstantBufferDataByteSizeOfEachPerPassObject();
	}

	std::array<Entity, SceneManager::MAX_ENTITIES>& WorldManager::GetEntities() {
		return mSceneManager.GetEntities();
	}

	const Vector3& WorldManager::GetPlayerCenter() const {
		return mPlayer.GetCenter();
	}

	Entity& WorldManager::GetPlayerEntity() {
		return mSceneManager.GetPlayerEntity();
	}

	std::vector<const Mesh*> WorldManager::GetMeshesToLoad() {
		return mSceneManager.GetMeshesToLoad();
	}

	void WorldManager::PrepareForUpdate() {
		mSceneManager.PrepareForUpdate();
	}

#pragma endregion

#pragma region Private
	bool WorldManager::LoadScene() {
		if (!mSceneManager.LoadScene()) { return false; }

		mPlayer.SetEntity(mSceneManager.GetPlayerEntity());

		return true;
	}
#pragma endregion
}