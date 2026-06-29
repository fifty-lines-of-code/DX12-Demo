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
	
	bool Entity::Initialize(
		const EntityBlueprint& entityBlueprint,
		uint32_t id,
		EngineResources::Mesh* mesh
	) noexcept {
		SetIsDirty(true);
		SetIsActive(true);
		SetID(id);
		SetEntityType(entityBlueprint.EntityType);
		SetCenter(entityBlueprint.Center);
		SetScale(entityBlueprint.Scale);
		SetBasisVectors(entityBlueprint.BasisVectors);
		SetSurfaceNormal(entityBlueprint.SurfaceNormal);
		SetIsStatic(entityBlueprint.IsStatic);
		SetMesh(mesh);

		// update all submeshes
		for (int i = 0; i < entityBlueprint.ActiveSubMeshCount; ++i) {
			const EntitySubMeshBlueprint& subMeshBlueprint = entityBlueprint.EntitySubMeshBlueprints[i];
			SetSubMeshMaterialAndTexture(
				i,
				subMeshBlueprint.MaterialType,
				subMeshBlueprint.TextureID
			);
		}

		return true;
	}

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
			"SubMesh Index is Incorrect, Bad things will happen!"
		);

		uint8_t index = subMeshIndex < EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH 
			? subMeshIndex : 0;

		// store Material ID;
		destinationBufferData.MaterialID = mRenderData.SubMeshMaterials[index].MaterialID;

		// store Texture ID
		destinationBufferData.TextureID = mRenderData.SubMeshMaterials[index].TextureID;
	}

	uint32_t Entity::GetID() const noexcept { return mID; }

	bool Entity::GetIsStatic() const noexcept { return mIsStatic; }

	Vector3 Entity::GetCenter() const noexcept { return mTransformData.Center; }

	Vector3 Entity::GetScale() const noexcept { return mTransformData.Scale; }

	Vector3 Entity::GetSurfaceNormal() const noexcept { 
		return mRenderData.SurfaceNormal; 
	}

	// thus this returns an ID
	const EngineResources::Mesh* Entity::GetMesh() const noexcept { return mMesh; }

	EnginePhysics::PhysicsBody& Entity::GetPhysicsBody() noexcept { return mPhysicsBody; }

	const AABB& Entity::GetAABB() const noexcept { return mPhysicsBody.WorldAABB; }

	EntityTransformData& Entity::GetTransformData() noexcept { return mTransformData; }

	bool Entity::GetIsDirty() const { return mIsDirty; }

	void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

	bool Entity::GetIsActive() const noexcept { return mIsActive; }

	bool Entity::GetIsTerrainOrFloor() const noexcept {
		return
			mEntityType == EntityType::TERRAIN ||
			mEntityType == EntityType::FLOOR;
	}

	EntityType Entity::GetEntityType() const noexcept { return mEntityType; }


	bool Entity::IsSubMeshAtIndexRenderingAMirror(uint32_t subMeshIndex) const noexcept {
		if (mEntityType != EntityType::MIRROR) { return false; }

		ENGINE_ASSERT(
			subMeshIndex < EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH,
			"SubMesh Index is Incorrect, Bad things will happen!"
		);

		uint8_t index =
			subMeshIndex >= EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH ?
			0 : subMeshIndex;

		return mRenderData.SubMeshMaterials[index].IsUsingMirrorMaterial;
	}

#pragma region Private

	void Entity::SetID(uint32_t id) { mID = id; }

	void Entity::SetIsStatic(bool isStatic) { mIsStatic = isStatic; }

	//TODO: store the mesh ID instead of a pointer indirection for efficiency
	void Entity::SetMesh(EngineResources::Mesh* mesh) {
		mMesh = mesh;

		mPhysicsBody.LocalAABB.Min = mesh->GetLocalMin();
		mPhysicsBody.LocalAABB.Max = mesh->GetLocalMax();

		mPhysicsBody.UpdateWorldAABB(mRenderData.WorldMatrix);
	}

	void Entity::SetIsActive(bool isActive) noexcept { mIsActive = isActive; }

	void Entity::SetCenter(const Vector3& center) noexcept { 
		mTransformData.Center = center;
	}

	void Entity::SetScale(const Vector3& scale) noexcept { 
		mTransformData.Scale = scale; 
	}

	void Entity::SetBasisVectors(const BasisVectors& basisVectors) noexcept {
		mTransformData.BasisVectors = basisVectors;
		mRenderData.RebuildWorldMatrix(
			mTransformData.Center,
			mTransformData.Scale,
			basisVectors
		);
	}

	void Entity::SetSurfaceNormal(const Vector3& surfaceNormal) noexcept {
		mRenderData.SurfaceNormal = surfaceNormal;
	}

	void Entity::SetEntityType(EntityType entityType) noexcept {
		mEntityType = entityType;
	}


	void Entity::SetSubMeshMaterialAndTexture(
		uint8_t subMeshIndex,
		EngineResources::MaterialType material,
		EngineResources::TextureID texture
	) noexcept 
	{
		ENGINE_ASSERT(mMesh != nullptr, "Mesh should NOT be nullptr here");
		if (mMesh == nullptr) { return; }

		ENGINE_ASSERT(
			subMeshIndex < EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH,
			"SubMesh Index is Incorrect, Bad things will happen!"
		);

		uint8_t index =
			subMeshIndex >= EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH ?
			0 : subMeshIndex;

		mRenderData.SubMeshMaterials[index].MaterialID = (uint8_t)material;
		mRenderData.SubMeshMaterials[index].IsUsingMirrorMaterial =
			material == EngineResources::MaterialType::MIRROR;
		mRenderData.SubMeshMaterials[index].TextureID = (uint8_t)texture;
	}
#pragma endregion
}