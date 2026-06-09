#include "Entity.h"

#include <cmath>
#include <DirectXMath.h>
#include "Mesh/Mesh.h"

namespace Engine {

	Entity::Entity() : 
		Entity(-1, Vector3(), Vector3(), true, false)
	{}

	Entity::Entity(uint32_t Id, Vector3 center, Vector3 scale, bool isStatic, bool isActive) :
		mID(Id),
		mPhysicsBody(center, scale),
		mIsStatic(isStatic),
		mMesh(nullptr),
		mIsDirty(true),
		mIsActive(false),
		mMaterialType(EngineResources::MaterialType::Invalid)
	{}

	Entity::~Entity() {}

	void Entity::SetID(uint32_t id) { mID = id; }

	uint32_t Entity::GetID() const { return mID; }

	void Entity::SetIsStatic(bool isStatic) { mIsStatic = isStatic; }

	bool Entity::GetIsStatic() const { return mIsStatic; }

	void Entity::SetMesh(const Mesh* mesh) {
		mMesh = mesh;

		mPhysicsBody.LocalAABB.Min = mesh->GetLocalMin();
		mPhysicsBody.LocalAABB.Max = mesh->GetLocalMax();

		mPhysicsBody.UpdateProductionTransforms();
	}

	const Mesh* Entity::GetMesh() const { return mMesh; }

	void Entity::Update(float stickX, float stickY, float deltaTime, float speed) {
		if (mIsDirty) { mPhysicsBody.UpdateProductionTransforms(); }
	}

	void Entity::CopyToDestinationConstantBufferDataTransposed(EntityConstantBufferData& bufferData) {
		// store world transpose
		DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(&mPhysicsBody.WorldMatrix.AsXMFLOAT4X4())
		);
		DirectX::XMStoreFloat4x4(
			&bufferData.World.AsXMFLOAT4X4(),
			worldTranspose
		);

		// store Material ID
		bufferData.MaterialID = (uint16_t)mMaterialType;
	}

	EnginePhysics::PhysicsBody& Entity::GetPhysicsBody() { return mPhysicsBody; }

	const AABB& Entity::GetAABB() const { return mPhysicsBody.WorldAABB; }

	bool Entity::GetIsDirty() const { return mIsDirty; }

	void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

	bool Entity::GetIsActive() const noexcept { return mIsActive; }

	void Entity::SetIsActive(bool isActive) noexcept { mIsActive = isActive; }

	void Entity::SetScale(Vector3 scale) { mPhysicsBody.Scale = scale; }

	void Entity::SetMaterialType(EngineResources::MaterialType type) { 
		mMaterialType = type; 
	}
}