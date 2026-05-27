#include "OctTreeNode.h"

#include "../../Entities/Entity.h"

namespace Engine {

	OctTreeNode::OctTreeNode() :
		mCenter(Vector3()),
		mHalfWidth(0.f),
		mStartIndexOfChildNodes(UINT32_MAX)
	{}

	OctTreeNode::OctTreeNode(Vector3 center, float halfWidth) :
		mCenter(center), 
		mHalfWidth(halfWidth),
		mStartIndexOfChildNodes(UINT32_MAX) {}

	OctTreeNode::~OctTreeNode() = default;

	const Vector3& OctTreeNode::GetCenter() const { return mCenter; }

	float OctTreeNode::GetHalfWidth() const { return mHalfWidth; }

	const Vector3& OctTreeNode::GetMin() const { return mNodeMin; }

	const Vector3& OctTreeNode::GetMax() const { return mNodeMax; }

	void OctTreeNode::SetCenter(const Vector3& center) { mCenter = center; }

	void OctTreeNode::SetHalfWidth(float halfWidth) { mHalfWidth = halfWidth; }

	void OctTreeNode::SetBounds(Vector3 min, Vector3 max) {
		mNodeMin = min;
		mNodeMax = max;
	}

	void OctTreeNode::SetStartIndexOfChildNodes(uint32_t startingIndex) {
		mStartIndexOfChildNodes = startingIndex;
	}

	void OctTreeNode::UpdateEntities(const Entity* entity) {
		mEntities.push_back(entity);
	}

	uint32_t OctTreeNode::GetStartIndexOfChildNodes() const { return mStartIndexOfChildNodes; }
}