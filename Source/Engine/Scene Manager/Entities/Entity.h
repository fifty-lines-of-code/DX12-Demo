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
	Entity(uint32_t Id);
	~Entity();

	uint32_t GetID() const;
	void SetMesh(const Mesh* mesh);
	const Mesh* GetMesh() const;
	void Update(const DirectX::XMFLOAT4X4* viewProj);

	DirectX::XMFLOAT4X4 GetConstantBufferDataTransposeIfNecessray();
	const EntityAABBMinMax GetAABBMinMax() const;
	bool GetIsDirty() const;
	void SetIsDirty(bool dirty);

private:
	uint32_t mID;
	DirectX::XMFLOAT3 mCenter = DirectX::XMFLOAT3(0.f, 0.f, 2.f);
	float mScale = 1.f;
	const Mesh* mMesh;
	EntityConstantBufferData mConstantBufferData;
	bool mIsDirty = true;
};