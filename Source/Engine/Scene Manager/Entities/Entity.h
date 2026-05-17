#pragma once

#include "PerPassAndPerEntityConstantBufferData.h"

class Mesh;

// width because our aabb will always be a cube
// maybe we can optimize in the future
struct EntityAABBMinMax {
	DirectX::XMFLOAT3 Min;
	DirectX::XMFLOAT3 Max;
	float halfWidth;
};

class Entity {
public:
	Entity(uint32_t Id, DirectX::XMFLOAT3 center, float scaleX, float scaleY, float scaleZ);
	~Entity();

	uint32_t GetID() const;
	void SetMesh(const Mesh* mesh);
	const Mesh* GetMesh() const;

	void Update(float stickX, float stickY, float deltaTime, float speed);

	DirectX::XMFLOAT4X4 GetConstantBufferDataTransposed();
	const EntityAABBMinMax GetAABBMinMax() const;

	bool GetIsDirty() const;
	void SetIsDirty(bool dirty);

	DirectX::XMFLOAT3 GetCenter() const;
	void SetCenter(DirectX::XMFLOAT3 center);

private:
	uint32_t mID;
	DirectX::XMFLOAT3 mCenter;
	float mScaleX;
	float mScaleY;
	float mScaleZ;
	const Mesh* mMesh;
	EntityConstantBufferData mConstantBufferData;
	bool mIsDirty = true;

private:
	void CalculateWorldMatrix();	
};