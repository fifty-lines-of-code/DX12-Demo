#include "Entity.h"

#include <cmath>
#include <DirectXMath.h>
#include "Mesh/Mesh.h"

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
	) :	mPhysicsBody(center, scale),
		mMesh(nullptr),
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
	void Entity::SetMesh(Mesh* mesh) {
		mMesh = mesh;

		mPhysicsBody.LocalAABB.Min = mesh->GetLocalMin();
		mPhysicsBody.LocalAABB.Max = mesh->GetLocalMax();

		mPhysicsBody.UpdateProductionTransforms();
	}

	// thus this returns an ID
	const Mesh* Entity::GetMesh() const { return mMesh; }

	void Entity::Update(float stickX, float stickY, float deltaTime, float speed) {
		if (mIsDirty) { mPhysicsBody.UpdateProductionTransforms(); }
	}

	void Entity::CopyToDestinationEntityConstantBufferDataTransposed(EntityConstantBufferData& bufferData) {
		// store world transpose
		DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(&mPhysicsBody.WorldMatrix.AsXMFLOAT4X4())
		);
		DirectX::XMStoreFloat4x4(
			&bufferData.World.AsXMFLOAT4X4(),
			worldTranspose
		);
	}
	
	void Entity::CopyToDestinationSubMeshConstantBufferDataTransposed(
		uint8_t subMeshId,
		EntitySubMeshConstantBufferData& destinationBufferData
	) {
		const SubMesh& subMesh = mMesh->GetSubMeshAtIndex(subMeshId);

		// store Material ID
		bool isValid = subMesh.MaterialID < (uint16_t)EngineResources::MaterialType::COUNT;
		destinationBufferData.MaterialID = isValid ? subMesh.MaterialID : 0;

		// store Texture ID
		destinationBufferData.TextureID = subMesh.TextureID;
	}

	EnginePhysics::PhysicsBody& Entity::GetPhysicsBody() { return mPhysicsBody; }

	const AABB& Entity::GetAABB() const { return mPhysicsBody.WorldAABB; }

	bool Entity::GetIsDirty() const { return mIsDirty; }

	void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

	bool Entity::GetIsActive() const noexcept { return mIsActive; }

	void Entity::SetIsActive(bool isActive) noexcept { mIsActive = isActive; }

	void Entity::SetScale(Vector3 scale) { mPhysicsBody.Scale = scale; }

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
		if (mMesh == nullptr) { return; }

		uint32_t materialID = (uint32_t)material >= 
			(uint32_t)EngineResources::MaterialType::COUNT ?
			0 : (uint32_t)material;

		uint32_t textureID = (uint32_t)texture >= 
			(uint32_t)EngineResources::TextureID::COUNT ?
			0 : (uint32_t)texture;

		mMesh->UpdateSubMeshAtIndex(subMeshIndex, materialID, textureID);
	}
}