#include "Entity.h"

#include "Mesh/Mesh.h"

Entity::Entity(uint32_t Id) : mID(Id), mMesh(nullptr) {}

Entity::~Entity() {}

uint32_t Entity::GetID() const {
	return mID;
}

void Entity::SetMesh(const Mesh* mesh) {
	this->mMesh = mesh;
}

const Mesh* Entity::GetMesh() const {
	return mMesh;
}

void Entity::Update(const DirectX::XMFLOAT4X4* viewProj) {
	DirectX::XMFLOAT4X4 identity = MathHelper::Identity4x4();

	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&identity);
	DirectX::XMStoreFloat4x4(
		&mConstantBufferData.World, 
		world
	);

	//todo: for now we will set this once
	// but later only set isDirty to true if we have't changed any state
	static bool hasRunOnce = false;
	if (!hasRunOnce) {
		mIsDirty = true;
		hasRunOnce = false;
	}
}

DirectX::XMFLOAT4X4 Entity::GetConstantBufferDataTransposeIfNecessray() {
	DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
		DirectX::XMLoadFloat4x4(&mConstantBufferData.World)
	);
	DirectX::XMFLOAT4X4 worldTranspose44;
	DirectX::XMStoreFloat4x4(&worldTranspose44, worldTranspose);
	return worldTranspose44;
}

const EntityAABBMinMax Entity::GetAABBMinMax() const {
	float halfSize = mScale *.5f;

	DirectX::XMFLOAT3 min = DirectX::XMFLOAT3(mCenter.x - halfSize, mCenter.y - halfSize, mCenter.z - halfSize);
	DirectX::XMFLOAT3 max = DirectX::XMFLOAT3(mCenter.x + halfSize, mCenter.y + halfSize, mCenter.z + halfSize);

	return EntityAABBMinMax{min, max , halfSize};
}

bool Entity::GetIsDirty() const {
	return mIsDirty;
}

void Entity::SetIsDirty(bool dirty) {
	mIsDirty = dirty;
}