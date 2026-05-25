#include "Entity.h"

#include <cmath>
#include <DirectXMath.h>
#include "Mesh/Mesh.h"

Entity::Entity(uint32_t Id, Engine::Vector3 center, float scaleX, float scaleY, float scaleZ) :
	mID(Id),
	mCenter(center),
	mScaleX(scaleX),
	mScaleY(scaleY),
	mScaleZ(scaleZ),
	mMesh(nullptr)
{
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

void Entity::CopyToDestinationConstantBufferDataTransposed(Engine::Matrix4x4* destination) {
	DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(
		DirectX::XMLoadFloat4x4(&mConstantBufferData.World.AsXMFLOAT4X4())
	);
	DirectX::XMStoreFloat4x4(
		&destination->AsXMFLOAT4X4(),
		worldTranspose
	);
}

const EntityAABBMinMax Entity::GetAABBMinMax() const {
	float halfSize = mScaleX *.5f;

	Engine::Vector3 min = Engine::Vector3(mCenter.x - halfSize, mCenter.y - halfSize, mCenter.z - halfSize);
	Engine::Vector3 max = Engine::Vector3(mCenter.x + halfSize, mCenter.y + halfSize, mCenter.z + halfSize);

	return EntityAABBMinMax{min, max , halfSize};
}

bool Entity::GetIsDirty() const { return mIsDirty; }

void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

Engine::Vector3 Entity::GetCenter() const { return mCenter; }

void Entity::SetCenter(Engine::Vector3 center) { mCenter = center; }

void Entity::SetBasisVectors(const Engine::BasisVectors* const basisVectors) { 
	mBasisVectors.forward = basisVectors->forward;
	mBasisVectors.up = basisVectors->up;
	mBasisVectors.right = basisVectors->right;
}

void Entity::CalculateWorldMatrix() {
	// todo: Use DX methods to build SRT and then W
	// DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(mScaleX, mScaleY, mScaleZ);
	Engine::Matrix4x4 scale;
	Engine::Matrix4x4 rotation;
	Engine::Matrix4x4 translation;

	// Set Scale
	scale.m[0][0] = mScaleX;
	scale.m[1][1] = mScaleY;
	scale.m[2][2] = mScaleZ;

	// Set Rotation
	// Row 0: Right
	rotation.m[0][0] = mBasisVectors.right.x;
	rotation.m[0][1] = mBasisVectors.right.y;
	rotation.m[0][2] = mBasisVectors.right.z;
	rotation.m[0][3] = 0.f;

	// Row 1: Up
	rotation.m[1][0] = mBasisVectors.up.x;
	rotation.m[1][1] = mBasisVectors.up.y;
	rotation.m[1][2] = mBasisVectors.up.z;
	rotation.m[1][3] = 0.f;

	// Row 2: Forward
	rotation.m[2][0] = mBasisVectors.forward.x;
	rotation.m[2][1] = mBasisVectors.forward.y;
	rotation.m[2][2] = mBasisVectors.forward.z;
	rotation.m[2][3] = 0.f;

	// Set Translation
	translation.m[3][0] = mCenter.x;
	translation.m[3][1] = mCenter.y;
	translation.m[3][2] = mCenter.z;
	translation.m[3][3] = 1.0f;

	// lets read and write to our Matrix4x4 as an XMFLOAT4x4 so that
	// we get access to fast SIMD math operations
	// from my understand there should be 0 performance penalty for this cast
	// and it allows us to keep our code clean, without having to use too many 
	// XMMatrix* methods

	DirectX::XMMATRIX scaleRotation = DirectX::XMMatrixMultiply(
		DirectX::XMLoadFloat4x4(&scale.AsXMFLOAT4X4()),
		DirectX::XMLoadFloat4x4(&rotation.AsXMFLOAT4X4())
	);

	DirectX::XMMATRIX world = DirectX::XMMatrixMultiply(
		scaleRotation, DirectX::XMLoadFloat4x4(&translation.AsXMFLOAT4X4()));

	DirectX::XMStoreFloat4x4(
		&mConstantBufferData.World.AsXMFLOAT4X4(),
		world
	);
}