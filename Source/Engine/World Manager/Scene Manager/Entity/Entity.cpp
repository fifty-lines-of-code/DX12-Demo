#include "Entity.h"

#include <cmath>
#include <DirectXMath.h>
#include "../../../../Helper/Logger.h"

namespace Engine::EngineWorld {

	Entity::Entity() : 
		Entity(-1, Vector3(), Vector3(), true, false)
	{}

	Entity::Entity(
		uint32_t Id, 
		Vector3 center, 
		Vector3 scale, 
		bool isStatic, 
		bool isActive
	) :	mMesh(nullptr),
		mID(Id),
		mEntityType(EntityType::INVALID),
		mIsStatic(isStatic),
		mIsDirty(true),
		mIsActive(false)
	{}

	Entity::~Entity() {}

	void Entity::SetID(uint32_t id) { mID = id; }

	uint32_t Entity::GetID() const { return mID; }

	void Entity::SetIsStatic(bool isStatic) { mIsStatic = isStatic; }

	bool Entity::GetIsStatic() const { return mIsStatic; }

	//TODO: store the mesh ID instead of a pointer indirection for efficiency
	void Entity::SetMesh(EngineResources::Mesh* mesh) {
		mMesh = mesh;

		mPhysicsBody.LocalAABB.Min = mesh->GetLocalMin();
		mPhysicsBody.LocalAABB.Max = mesh->GetLocalMax();

		mPhysicsBody.UpdateWorldAABB(mRenderData.WorldMatrix);
	}

	// thus this returns an ID
	const EngineResources::Mesh* Entity::GetMesh() const { return mMesh; }

	void Entity::Update(float stickX, float stickY, float deltaTime, float speed) {
		if (mIsDirty) { 
			mRenderData.RebuildWorldMatrix(
				mTransformData.Center,
				mTransformData.Scale,
				mTransformData.BasisVectors
			);
			mPhysicsBody.UpdateWorldAABB(
				mRenderData.WorldMatrix
			); 
		}
	}

	void Entity::CopyToDestinationEntityConstantBufferDataTransposed(EntityConstantBufferData& bufferData) {
		// store world transpose
		DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(&mRenderData.WorldMatrix.AsXMFLOAT4X4())
		);
		DirectX::XMStoreFloat4x4(
			&bufferData.World.AsXMFLOAT4X4(),
			worldTranspose
		);
	}
	
	void Entity::CopyToDestinationSubMeshConstantBufferData(
		uint8_t subMeshIndex,
		EntitySubMeshConstantBufferData& destinationBufferData
	) {
		ENGINE_ASSERT(
			subMeshIndex < EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH,
			L"SubMesh Index is Incorrect, Bad things will happen!"
		);

		uint8_t index = subMeshIndex < EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH 
			? subMeshIndex : 0;

		// store Material ID;
		destinationBufferData.MaterialID = mRenderData.SubMeshMaterials[index].MaterialID;

		// store Texture ID
		destinationBufferData.TextureID = mRenderData.SubMeshMaterials[index].TextureID;
	}

	EnginePhysics::PhysicsBody& Entity::GetPhysicsBody() noexcept { return mPhysicsBody; }

	const AABB& Entity::GetAABB() const noexcept { return mPhysicsBody.WorldAABB; }

	EntityTransformData& Entity::GetTransformData() noexcept { return mTransformData; }

	bool Entity::GetIsDirty() const { return mIsDirty; }

	void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

	bool Entity::GetIsActive() const noexcept { return mIsActive; }

	void Entity::SetIsActive(bool isActive) noexcept { mIsActive = isActive; }

	void Entity::SetCenter(Vector3 center) { 
		mTransformData.Center = center;
	}

	void Entity::SetScale(Vector3 scale) { 
		mTransformData.Scale = scale; 
	}

	bool Entity::GetIsTerrainOrFloor() const noexcept { 
		return 
			mEntityType == EntityType::TERRAIN ||
			mEntityType == EntityType::FLOOR; 
	}

	void Entity::SetEntityType(EntityType entityType) noexcept {
		mEntityType = entityType;
	}

	EntityType Entity::GetEntityType() const noexcept { return mEntityType; }

	void Entity::SetSubMeshMaterialAndTexture(
		uint8_t subMeshIndex,
		EngineResources::MaterialType material,
		EngineResources::TextureID texture
	) noexcept 
	{
		ENGINE_ASSERT(mMesh != nullptr, L"Mesh should NOT be nullptr here");
		if (mMesh == nullptr) { return; }

		ENGINE_ASSERT(
			subMeshIndex < EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH,
			L"SubMesh Index is Incorrect, Bad things will happen!"
		);

		uint8_t index =
			subMeshIndex >= EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH ?
			0 : subMeshIndex;

		mRenderData.SubMeshMaterials[index].MaterialID = (uint8_t)material;
		mRenderData.SubMeshMaterials[index].TextureID = (uint8_t)texture;
	}
}