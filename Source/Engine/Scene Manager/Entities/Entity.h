#pragma once

#include "../../Math/BasisVectors.h"
#include "PerPassAndPerEntityConstantBufferData.h"

class Mesh;

// width because our aabb will always be a cube
// maybe we can optimize in the future
struct EntityAABBMinMax {
	Engine::Vector3 Min = Engine::Vector3::Zero();
	Engine::Vector3 Max = Engine::Vector3::Zero();
	float halfWidth;
};

class Entity {
public:
	Entity(
		uint32_t Id, 
		Engine::Vector3 center,
		float scaleX, 
		float scaleY,
		float scaleZ
	);
	~Entity();

	uint32_t GetID() const;
	void SetMesh(const Mesh* mesh);
	const Mesh* GetMesh() const;

	void Update(float stickX, float stickY, float deltaTime, float speed);

	void CopyConstantBufferDataTransposed(Engine::Matrix4x4* destination);
	const EntityAABBMinMax GetAABBMinMax() const;

	bool GetIsDirty() const;
	void SetIsDirty(bool dirty);

	Engine::Vector3 GetCenter() const;
	void SetCenter(Engine::Vector3 center);

	void SetBasisVectors(const Engine::BasisVectors* const basisVectors);

private:
	uint32_t mID;
	Engine::Vector3 mCenter;
	float mScaleX;
	float mScaleY;
	float mScaleZ;
	Engine::BasisVectors mBasisVectors;
	const Mesh* mMesh;
	EntityConstantBufferData mConstantBufferData;
	bool mIsDirty = true;

private:
	void CalculateWorldMatrix();	
};