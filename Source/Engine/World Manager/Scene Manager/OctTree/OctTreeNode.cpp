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

	uint32_t OctTreeNode::GetStartIndexOfChildren() const { 
		return mStartIndexOfChildNodes;
	}

	const std::vector<uint32_t>& OctTreeNode::GetStaticEntities() const {
		return mStaticEntities;
	}

	const std::vector<uint32_t>& OctTreeNode::GetDynamicEntities() const {
		return mDynamicEntities;
	}

	void OctTreeNode::SetCenter(const Vector3& center) { mCenter = center; }

	void OctTreeNode::SetHalfWidth(float halfWidth) { mHalfWidth = halfWidth; }

	void OctTreeNode::SetBounds(Vector3 min, Vector3 max) {
		mAABB = AABB(min, max);
	}

	void OctTreeNode::SetStartIndexOfChildNodes(uint32_t startingIndex) {
		mStartIndexOfChildNodes = startingIndex;
	}

	void OctTreeNode::UpdateStaticEntities(uint32_t entityIndex) {
		mStaticEntities.push_back(entityIndex);
	}

	void OctTreeNode::UpdateDynamicEntities(uint32_t entityIndex) {
		mDynamicEntities.push_back(entityIndex);
	}

	void OctTreeNode::ClearDynamicEntities() {
		mDynamicEntities.clear();
	}
}