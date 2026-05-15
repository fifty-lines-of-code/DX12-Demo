#include "Entity.h"

#include "Mesh/Mesh.h"

Entity::Entity() : mMesh(nullptr) {}

Entity::~Entity() {}

void Entity::SetMesh(const Mesh* mesh) {
	this->mMesh = mesh;
}

const Mesh* Entity::GetMesh() const {
	return mMesh;
}

void Entity::Update(DirectX::XMFLOAT4X4 viewProj) {
	DirectX::XMFLOAT4X4 identity = MathHelper::Identity4x4();

	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&identity);
	DirectX::XMMATRIX xmViewProj = DirectX::XMLoadFloat4x4(&viewProj);
	DirectX::XMMATRIX worldViewProj = world * xmViewProj;
	DirectX::XMStoreFloat4x4(
		&mConstantBufferData.WorldViewProjection, 
		XMMatrixTranspose(worldViewProj)
	);
}

void* Entity::GetConstantBufferData() {
	return &mConstantBufferData.WorldViewProjection;
}

const EntityAABBMinMax Entity::GetAABBMinMax() const {
	//todo: calculate the AABB min max based on position and scale
	
	// for now return the AABB min max of a unit cube centered at the origin
	DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float halfSize = 1.0 / 2;

	DirectX::XMFLOAT3 min = DirectX::XMFLOAT3(center.x - halfSize, center.y - halfSize, center.z - halfSize);
	DirectX::XMFLOAT3 max = DirectX::XMFLOAT3(center.x + halfSize, center.y + halfSize, center.z + halfSize);

	return EntityAABBMinMax{min, max , halfSize};
}