#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>

class Entity;
struct OctTreeNode;
struct EntityAABBMinMax;

struct OctTreeNode {
	static constexpr uint32_t NUMBER_OF_CHILDREN = 8;

public:
	OctTreeNode(DirectX::XMFLOAT3 c, float hw)
		: mCenter(c), mHalfWidth(hw) {
		for (int i = 0; i < 8; ++i) { mChildren[i] = nullptr; }
	}
	~OctTreeNode();

	DirectX::XMFLOAT3 GetCenter() const { return mCenter; }
	float GetHalfWidth() const { return mHalfWidth; }
	DirectX::XMFLOAT3 GetMin() const { return mNodeMin; }
	DirectX::XMFLOAT3 GetMax() const { return mNodeMax; }

	void SetBounds(DirectX::XMFLOAT3 min, DirectX::XMFLOAT3 max) {
		mNodeMin = min;
		mNodeMax = max;
	}

	OctTreeNode* GetChildAtIndex(int index) {
		if (index < 0 || index >= NUMBER_OF_CHILDREN) {
			return nullptr;
		}

		return mChildren[index].get();
	}

	void UpdateChildAtIndex(int i, std::unique_ptr<OctTreeNode> node) {
		mChildren[i] = std::move(node);
	}

	void UpdateEntities(const Entity* entity) {
		mEntities.push_back(entity);
	}

private:
	DirectX::XMFLOAT3 mCenter;
	DirectX::XMFLOAT3 mNodeMin;
	DirectX::XMFLOAT3 mNodeMax;
	float mHalfWidth;
	std::vector<const Entity*> mEntities;

	//std::array<std::unique_ptr<OctTreeNode>, 8> mChildren;
	// array throws incomplete type error because OctTreeNode is not 
	// fully defined at the point of declaration.
	// The fix: unique_ptr handles the "Incomplete Type" 
	// as long as the compiler knows it's a pointer.
	// Note: Some compilers require a custom destructor for this to work 
	// with unique_ptr inside the struct.
	std::unique_ptr<OctTreeNode> mChildren[NUMBER_OF_CHILDREN];
};

class OctTree {
public:
	OctTree(const DirectX::XMFLOAT3 center, float halfWidth);
	~OctTree();
	bool Insert(const Entity* entity);
	
private:
	// why depth of 3? Because our scene is small and we don't have many entities, so we don't need a very deep tree, but helps to learn the idea
	// in the real world it's usually between 5 and 8, but it really depends on the scene and the number of entities (or so copiolot syas)

	static constexpr uint32_t MAX_DEPTH = 3;
	std::unique_ptr<OctTreeNode> mRoot;

private:
	bool Insert_Internal(const Entity* entity, OctTreeNode* const node, uint32_t depth);
	void Subdivide(OctTreeNode* node, float childrenHalfWidth);

	bool DoesEntityFitInNode(EntityAABBMinMax entityAABB, float nodeHalfSize, DirectX::XMFLOAT3 nodeCenter, DirectX::XMFLOAT3 nodeMin, DirectX::XMFLOAT3 nodeMax);
	void CalculateMinAndMax(OctTreeNode* node);
};