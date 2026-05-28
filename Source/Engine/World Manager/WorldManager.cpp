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
		PrepareForNewFrame();

		// tell player to calculate it's new potential center
		mPlayer.CalculateNewPotentialCenter(
			deltaTime,
			inputSystem,
			cameraBasisVectors
		);

		// get all entities player is colliding with
		std::vector<const Entity*> collisions;
		collisions.reserve(8);

		mSceneManager.GetCollisionsWithPlayer(
			mSceneManager.GetPlayerEntity().GetPotentialAABB(),
			collisions
		);

		// todo:
		// crude right now, will build later
		// if collision, don't do anything
		// otherwise update player
		bool foundCollision = false;
		for (const Entity* entity: collisions) {
			// don't check with ourselves
			if (entity->GetID() == mSceneManager.GetPlayerEntity().GetID()) { continue; }

			foundCollision = true;
			break;
		}

		if (!foundCollision) {
			// then update player
			mPlayer.Update(
				deltaTime,
				inputSystem,
				cameraBasisVectors
			);
		}

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

	void WorldManager::PrepareForNewFrame() {
		mSceneManager.PrepareForNewFrame();
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