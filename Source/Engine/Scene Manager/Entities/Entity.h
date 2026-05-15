#pragma once

#include "EntityConstantBufferData.h"

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
	Entity();
	~Entity();

	void SetMesh(const Mesh* mesh);
	const Mesh* GetMesh() const;
	void Update(DirectX::XMFLOAT4X4 viewProj);

	void* GetConstantBufferData();
	const EntityAABBMinMax GetAABBMinMax() const;

private:
	const Mesh* mMesh;
	EntityConstantBufferData mConstantBufferData;
};