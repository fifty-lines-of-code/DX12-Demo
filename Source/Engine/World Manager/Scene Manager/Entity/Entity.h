#pragma once

#include "../../../Math/BasisVectors.h"
#include "PerPassAndPerEntityConstantBufferData.h"

class Mesh;

// width because our aabb will always be a cube
// maybe we can optimize in the future
struct AABB {
	Engine::Vector3 Min;
	Engine::Vector3 Max;

	AABB() : AABB(Engine::Vector3(), Engine::Vector3()) {}
	AABB(Engine::Vector3 min, Engine::Vector3 max) : Min(min), Max(max) {}
};

class Entity {
public:
	Entity();
	Entity(
		uint32_t Id, 
		Engine::Vector3 center,
		Engine::Vector3 scale,
		bool isStatic
	);
	~Entity();

	void SetID(uint32_t id);
	uint32_t GetID() const;

	void SetIsStatic(bool isStatic);
	bool GetIsStatic() const;

	void SetMesh(const Mesh* mesh);
	const Mesh* GetMesh() const;

	void Update(float stickX, float stickY, float deltaTime, float speed);

	void CopyToDestinationConstantBufferDataTransposed(Engine::Matrix4x4* destination);
	const AABB& GetAABB() const;
	const AABB& GetPotentialAABB() const;

	bool GetIsDirty() const;
	void SetIsDirty(bool dirty);

	const Engine::Vector3& GetCenter() const;
	void SetCenter(Engine::Vector3 center);

	void SetPotentialCenter(Engine::Vector3 potentialCenter);
	const Engine::Vector3& GetPotentialCenter();

	void SetScale(Engine::Vector3 scale);

	void SetBasisVectors(const Engine::BasisVectors* const basisVectors);

private:
	uint32_t mID;
	Engine::Vector3 mCenter;
	Engine::Vector3 mPotentialCenter;
	Engine::Vector3 mScale;
	bool mIsStatic;
	Engine::BasisVectors mBasisVectors;
	const Mesh* mMesh;
	EntityConstantBufferData mConstantBufferData;
	bool mIsDirty;
	AABB mLocalAABB;
	AABB mWorldAABB;
	AABB mPotentialWorldAABB;
	Engine::Matrix4x4 mPotentialWorldMatrix;

private:
	void CalculateWorldMatrix(Engine::Matrix4x4& world, Engine::Vector3& center);
	void CalculateAABB(Engine::Matrix4x4& world, AABB& aabb);
};