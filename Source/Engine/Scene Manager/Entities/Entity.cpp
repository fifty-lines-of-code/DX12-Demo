#include "Entity.h"

#include <cmath>
#include "Mesh/Mesh.h"

Entity::Entity(uint32_t Id, DirectX::XMFLOAT3 center, float scaleX, float scaleY, float scaleZ) :
	mID(Id), 
	mCenter(center),
	mScaleX(scaleX),
	mScaleY(scaleY),
	mScaleZ(scaleZ),
	mMesh(nullptr) {
	CalculateWorldMatrix();
}

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

void Entity::Update(float stickX, float stickY, float deltaTime, float speed) {
	if (mIsDirty) {
		// update the World matrix
		CalculateWorldMatrix();
	}
}

DirectX::XMFLOAT4X4 Entity::GetConstantBufferDataTransposed() {
	DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
		DirectX::XMLoadFloat4x4(&mConstantBufferData.World)
	);
	DirectX::XMFLOAT4X4 worldTranspose44;
	DirectX::XMStoreFloat4x4(&worldTranspose44, worldTranspose);
	return worldTranspose44;
}

const EntityAABBMinMax Entity::GetAABBMinMax() const {
	float halfSize = mScaleX *.5f;

	DirectX::XMFLOAT3 min = DirectX::XMFLOAT3(mCenter.x - halfSize, mCenter.y - halfSize, mCenter.z - halfSize);
	DirectX::XMFLOAT3 max = DirectX::XMFLOAT3(mCenter.x + halfSize, mCenter.y + halfSize, mCenter.z + halfSize);

	return EntityAABBMinMax{min, max , halfSize};
}

bool Entity::GetIsDirty() const { return mIsDirty; }

void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

DirectX::XMFLOAT3 Entity::GetCenter() const { return mCenter; }

void Entity::SetCenter(DirectX::XMFLOAT3 center) { mCenter = center; }

void Entity::SetBasisVectors(const BasisVectors* const basisVectors) { 
	mBasisVectors.forward = basisVectors->forward;
	mBasisVectors.up = basisVectors->up;
	mBasisVectors.right = basisVectors->right;
}

void Entity::CalculateWorldMatrix() {
	// todo: Use DX methods to build SRT and then W
	// DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(mScaleX, mScaleY, mScaleZ);
	DirectX::XMFLOAT4X4 scale = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 rotation = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 translation = MathHelper::Identity4x4();

	// Set Scale
	scale.m[0][0] = mScaleX;
	scale.m[1][1] = mScaleY;
	scale.m[2][2] = mScaleZ;

	// Set Rotation
	// row 0 mapped to right
	rotation.m[0][0] = mBasisVectors.right.x;
	rotation.m[0][1] = mBasisVectors.right.y;
	rotation.m[0][2] = mBasisVectors.right.z;

	// Row 1: mapped to up
	rotation.m[1][0] = mBasisVectors.up.x;
	rotation.m[1][1] = mBasisVectors.up.y;
	rotation.m[1][2] = mBasisVectors.up.z;

	// Row 2: Mapped directly to your Forward vector
	rotation.m[2][0] = mBasisVectors.forward.x;
	rotation.m[2][1] = mBasisVectors.forward.y;
	rotation.m[2][2] = mBasisVectors.forward.z;

	// Set Translation
	translation.m[3][0] = mCenter.x;
	translation.m[3][1] = mCenter.y;
	translation.m[3][2] = mCenter.z;
	translation.m[3][3] = 1.0f;

	DirectX::XMMATRIX scaleRotation = DirectX::XMMatrixMultiply(
		DirectX::XMLoadFloat4x4(&scale),
		DirectX::XMLoadFloat4x4(&rotation)
	);

	DirectX::XMMATRIX world = DirectX::XMMatrixMultiply(
		scaleRotation,
		DirectX::XMLoadFloat4x4(&translation)
	);

	DirectX::XMStoreFloat4x4(
		&mConstantBufferData.World,
		world
	);
}