#include "Entity.h"

#include <cmath>
#include <DirectXMath.h>
#include "Mesh/Mesh.h"

Entity::Entity() : Entity(-1, Engine::Vector3(), Engine::Vector3(), true) 
{}

Entity::Entity(uint32_t Id, Engine::Vector3 center, Engine::Vector3 scale, bool isStatic) :
	mID(Id),
	mCenter(center),
	mPotentialCenter(center),
	mScale(scale),
	mIsStatic(isStatic),
	mMesh(nullptr),
	mIsDirty(true)
{
	CalculateWorldMatrix(mConstantBufferData.World, mCenter);
}

Entity::~Entity() {}

void Entity::SetID(uint32_t id) { mID = id; }

uint32_t Entity::GetID() const {
	return mID;
}

void Entity::SetIsStatic(bool isStatic) { mIsStatic = isStatic; }

bool Entity::GetIsStatic() const { return mIsStatic; }

void Entity::SetMesh(const Mesh* mesh) {
	mMesh = mesh;
	mLocalAABB.Min = mesh->GetLocalMin();
	mLocalAABB.Max = mesh->GetLocalMax();

	CalculateWorldMatrix(mConstantBufferData.World, mCenter);
	CalculateAABB(mConstantBufferData.World, mWorldAABB);

	CalculateWorldMatrix(mPotentialWorldMatrix, mPotentialCenter);
	CalculateAABB(mPotentialWorldMatrix, mPotentialWorldAABB);
}

const Mesh* Entity::GetMesh() const {
	return mMesh;
}

void Entity::Update(float stickX, float stickY, float deltaTime, float speed) {
	if (mIsDirty) {
		// update the World matrix
		CalculateWorldMatrix(mConstantBufferData.World, mCenter);

		// update world aabb
		CalculateAABB(mConstantBufferData.World, mWorldAABB);
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

const AABB& Entity::GetAABB() const { return mWorldAABB; }

const AABB& Entity::GetPotentialAABB() const { return mPotentialWorldAABB; }

bool Entity::GetIsDirty() const { return mIsDirty; }

void Entity::SetIsDirty(bool dirty) { mIsDirty = dirty; }

const Engine::Vector3& Entity::GetCenter() const { return mCenter; }

void Entity::SetCenter(Engine::Vector3 center) { mCenter = center; }

void Entity::SetPotentialCenter(Engine::Vector3 potentialCenter) {
	mPotentialCenter = potentialCenter;

	CalculateWorldMatrix(mPotentialWorldMatrix, mPotentialCenter);
	CalculateAABB(mPotentialWorldMatrix, mPotentialWorldAABB);
}

const Engine::Vector3& Entity::GetPotentialCenter() { return mPotentialCenter; }

void Entity::SetScale(Engine::Vector3 scale) { mScale = scale; }

void Entity::SetBasisVectors(const Engine::BasisVectors* const basisVectors) { 
	mBasisVectors.forward = basisVectors->forward;
	mBasisVectors.up = basisVectors->up;
	mBasisVectors.right = basisVectors->right;
}

void Entity::CalculateWorldMatrix(Engine::Matrix4x4& world, Engine::Vector3& center) {
	// todo: Use DX methods to build SRT and then W
	// DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(mScaleX, mScaleY, mScaleZ);
	Engine::Matrix4x4 scale;
	Engine::Matrix4x4 rotation;
	Engine::Matrix4x4 translation;

	// Set Scale
	scale.m[0][0] = mScale.x;
	scale.m[1][1] = mScale.y;
	scale.m[2][2] = mScale.z;

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
	translation.m[3][0] = center.x;
	translation.m[3][1] = center.y;
	translation.m[3][2] = center.z;
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

	DirectX::XMMATRIX worldXM = DirectX::XMMatrixMultiply(
		scaleRotation, 
		DirectX::XMLoadFloat4x4(&translation.AsXMFLOAT4X4())
	);

	DirectX::XMStoreFloat4x4(
		&world.AsXMFLOAT4X4(),
		worldXM
	);
}

void Entity::CalculateAABB(Engine::Matrix4x4& world, AABB& aabb) {
	DirectX::XMMATRIX worldMatrix = DirectX::XMLoadFloat4x4(
		&world.AsXMFLOAT4X4()
	);
	Engine::Vector3& min = mLocalAABB.Min;
	Engine::Vector3& max = mLocalAABB.Max;

	const int numberOfVerticesInACube = 8;

	// generate all 8 vertices
	DirectX::XMVECTOR aabbVertices[numberOfVerticesInACube] = {
		// Bottom Ring
		DirectX::XMVectorSet(min.x, min.y, max.z, 1.0f), // 0: Bottom-Left-Back
		DirectX::XMVectorSet(max.x, min.y, max.z, 1.0f), // 1: Bottom-Right-Back
		DirectX::XMVectorSet(max.x, min.y, min.z, 1.0f), // 2: Bottom-Right-Front
		DirectX::XMVectorSet(min.x, min.y, min.z, 1.0f), // 3: Bottom-Left-Front

		// Top Ring
		DirectX::XMVectorSet(min.x, max.y, max.z, 1.0f), // 4: Top-Left-Back
		DirectX::XMVectorSet(max.x, max.y, max.z, 1.0f), // 5: Top-Right-Back
		DirectX::XMVectorSet(max.x, max.y, min.z, 1.0f), // 6: Top-Right-Front
		DirectX::XMVectorSet(min.x, max.y, min.z, 1.0f)  // 7: Top-Left-Front
	};

	DirectX::XMVECTOR vTransformed = DirectX::XMVector3TransformCoord(
		aabbVertices[0], 
		worldMatrix
	);
	DirectX::XMVECTOR vWorldMin = vTransformed;
	DirectX::XMVECTOR vWorldMax = vTransformed;

	// transform them using world matrix
	for (int i = 1; i < numberOfVerticesInACube; ++i) {
		// transform using the World matrix
		vTransformed = DirectX::XMVector3TransformCoord(
			aabbVertices[i], 
			worldMatrix
		);

		// update min and max
		vWorldMin = DirectX::XMVectorMin(vWorldMin, vTransformed);
		vWorldMax = DirectX::XMVectorMax(vWorldMax, vTransformed);
	}

	// save World Min Max
	DirectX::XMStoreFloat3(
		&aabb.Min.AsXMFLOAT3(),
		vWorldMin
	);

	DirectX::XMStoreFloat3(
		&aabb.Max.AsXMFLOAT3(),
		vWorldMax
	);
}