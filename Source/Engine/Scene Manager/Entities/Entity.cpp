#include "Entity.h"

#include <cmath>
#include "Mesh/Mesh.h"

Entity::Entity(uint32_t Id) :
	mID(Id), 
	mCenter(DirectX::XMFLOAT3(0.f, 0.f, 2.f)),
	mScale(1.f),
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
	// 1. Check if the player is pushing the stick
	if (stickX != 0.0f || stickY != 0.0f) {

		// 2. Calculate the length of the input vector to check for diagonals
		float lengthSquared = (stickX * stickX) + (stickY * stickY);

		float dirX = stickX;
		// Mapping stick Y input over to our 3D world Z axis for now until
		// we handle rotation
		// todo
		float dirZ = stickY; 

		// 3. Normalize direction
		if (lengthSquared > 1.0f) {
			// using the Quake3 copy-paste for the heck of it
			float oneOverLengthSquared = MathHelper::FastInverseSqrt(lengthSquared);
			// todo: switch back to oneOverLengthSquared = 1/lengthSquared;
			dirX *= oneOverLengthSquared;
			dirZ *= oneOverLengthSquared;
		}

		mCenter.x += dirX * speed * deltaTime;
		mCenter.z += dirZ * speed * deltaTime;

		// update the World matrix
		CalculateWorldMatrix();

		// set isDirty to true
		mIsDirty = true;
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

void Entity::CalculateWorldMatrix() {
	DirectX::XMFLOAT4X4 world = MathHelper::Identity4x4();
	// Row 0: Scale X
	world.m[0][0] = mScale;
	world.m[0][1] = 0.0f;
	world.m[0][2] = 0.0f;
	world.m[0][3] = 0.0f;

	// Row 1: Scale Y
	world.m[1][0] = 0.0f;
	world.m[1][1] = mScale;
	world.m[1][2] = 0.0f;
	world.m[1][3] = 0.0f;

	// Row 2: Scale Z
	world.m[2][0] = 0.0f;
	world.m[2][1] = 0.0f;
	world.m[2][2] = mScale;
	world.m[2][3] = 0.0f;

	// Row 3: Translation
	world.m[3][0] = mCenter.x;
	world.m[3][1] = mCenter.y;
	world.m[3][2] = mCenter.z;
	world.m[3][3] = 1.0f;

	mConstantBufferData.World = world;
}