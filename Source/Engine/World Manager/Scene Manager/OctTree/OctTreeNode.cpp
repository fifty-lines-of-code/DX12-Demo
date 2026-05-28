#include "OctTreeNode.h"


namespace Engine {

	OctTreeNode::OctTreeNode() :
		OctTreeNode::OctTreeNode(Vector3(), 0.f) 
	{}

	OctTreeNode::OctTreeNode(Vector3 center, float halfWidth) :
		mCenter(center), 
		mAABB(center, center),
		mHalfWidth(halfWidth),
		mStartIndexOfChildNodes(OctTreeNode::INVALID_START_INDEX) 
	{}

	OctTreeNode::~OctTreeNode() = default;

	const Vector3& OctTreeNode::GetCenter() const { return mCenter; }

	float OctTreeNode::GetHalfWidth() const { return mHalfWidth; }

	const AABB& OctTreeNode::GetAABB() const { return mAABB; }

	uint32_t OctTreeNode::GetStartIndexOfChildNodes() const { 
		return mStartIndexOfChildNodes;
	}

	const std::vector<const Entity*>& OctTreeNode::GetStaticEntities() const {
		return mStaticEntities;
	}

	void OctTreeNode::SetCenter(const Vector3& center) { mCenter = center; }

	void OctTreeNode::SetHalfWidth(float halfWidth) { mHalfWidth = halfWidth; }

	void OctTreeNode::SetBounds(Vector3 min, Vector3 max) {
		mAABB = AABB(min, max);
	}

	void OctTreeNode::SetStartIndexOfChildNodes(uint32_t startingIndex) {
		mStartIndexOfChildNodes = startingIndex;
	}

	void OctTreeNode::UpdateStaticEntities(const Entity* entity) {
		mStaticEntities.push_back(entity);
	}

	void OctTreeNode::UpdateDynamicEntities(const Entity* entity) {
		mDynamicEntities.push_back(entity);
	}

	void OctTreeNode::ClearDynamicEntities() {
		mDynamicEntities.clear();
	}
}