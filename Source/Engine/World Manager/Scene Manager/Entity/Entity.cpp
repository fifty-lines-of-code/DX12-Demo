#include "Entity.h"

#include <cmath>
#include <DirectXMath.h>
#include "Mesh/Mesh.h"

Entity::Entity() : Entity(-1, Engine::Vector3(), Engine::Vector3(), true) 
{}

Entity::Entity(uint32_t Id, Engine::Vector3 center, Engine::Vector3 scale, bool isStatic) :
	mID(Id),
	mPhysicsBody(center, scale),
	mIsStatic(isStatic),
	mMesh(nullptr),
	mIsDirty(true)
{}

Entity::~Entity() {}

void Entity::SetID(uint32_t id) { mID = id; }

uint32_t Entity::GetID() const {
	return mID;
}

void Entity::SetIsStatic(bool isStatic) { mIsStatic = isStatic; }

bool Entity::GetIsStatic() const { return mIsStatic; }

void Entity::SetMesh(const Mesh* mesh) {
	mMesh = mesh;

	mPhysicsBody.LocalAABB.Min = mesh->GetLocalMin();
	mPhysicsBody.LocalAABB.Max = mesh->GetLocalMax();

	mPhysicsBody.UpdateProductionTransforms();
}

const Mesh* Entity::GetMesh() const {
	return mMesh;
}

void Entity::Update(float stickX, float stickY, float deltaTime, float speed) {
	if (mIsDirty) {
		mPhysicsBody.UpdateProductionTransforms();
	}
}

void Entity::CopyToDestinationConstantBufferDataTransposed(Engine::Matrix4x4* destination) {
	DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
		DirectX::XMLoadFloat4x4(&mPhysicsBody.WorldMatrix.AsXMFLOAT4X4())
	);
	DirectX::XMStoreFloat4x4(
		&destination->AsXMFLOAT4X4(),
		worldTranspose
	);
}
Engine::EnginePhysics::PhysicsBody& Entity::GetPhysicsBody() { return mPhysicsBody; }

const Engine::AABB& Entity::GetAABB() const { return mPhysicsBody.WorldAABB; }

bool Entity::GetIsDirty() const { return mIsDirty; }

void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

void Entity::SetScale(Engine::Vector3 scale) { mPhysicsBody.Scale = scale; }