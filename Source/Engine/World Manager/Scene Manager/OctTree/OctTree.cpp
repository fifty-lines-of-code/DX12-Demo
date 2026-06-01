#include "OctTree.h"

#include "../Entity/Entity.h"
#include "../../../Math/GeometryHelper.h"

namespace Engine {

	OctTree::OctTree() : 
		mRoot(allNodes[0])
	{}

	OctTree::~OctTree() {}

	bool OctTree::Initialize(const Vector3& center, float halfWidth) {
		SetupRoot(center, halfWidth);
		IncrementNextAvailableStartIndex();

		return true;
	}

	bool OctTree::Insert(uint32_t entityIndex, const AABB& entityAABB, bool isStatic) {
		return Insert_Internal(entityIndex, entityAABB, isStatic, mRoot, 0);
	}

	void OctTree::ClearDynamicEntities() {
		for (uint32_t i = 0; i < mNextAvailableStartIndex; ++i) {
			allNodes[i].ClearDynamicEntities();
		}
	}

	void OctTree::GetPotentialCollisionsWithAABB(
		uint32_t playerID,
		const AABB& playerPotentialAABB,
		std::vector<uint32_t>& candidates
	) {
		GetPotentalCollisionsWithPlayer_Internal(
			0,
			playerPotentialAABB,
			candidates
		);
	}

#pragma region Private

	void OctTree::SetupRoot(const Vector3& center, float halfWidth) {
		mRoot.SetCenter(center);
		mRoot.SetHalfWidth(halfWidth);
		CalculateAndUpdateBoundsOfNode(mRoot);
	}

	void OctTree::IncrementNextAvailableStartIndex() {
		mNextAvailableStartIndex += OctTreeNode::NUMBER_OF_CHILDREN;
	}

	bool OctTree::Insert_Internal(uint32_t entityIndex, const AABB& entityAABB, bool isStatic, OctTreeNode& node, uint32_t depth) {

		// if this isn't the deepest level (leaf nodes)
		if (depth < OctTree::MAX_DEPTH) {
			double childrenHalfWidth = node.GetHalfWidth() * 0.5;

			// 1. Subdivide the node if it doesn't have children already
			// known by if start index == uint32_t::max
			// maybe we can find a better way to know if a node has been subdivided
			if (node.GetStartIndexOfChildren() == OctTreeNode::INVALID_START_INDEX) {
				Subdivide(node, (float)childrenHalfWidth);
			}

			uint32_t startIndex = node.GetStartIndexOfChildren();
			// 2. Find the child node to insert into
			for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
				uint32_t childIndex = startIndex + i;

				// theoretically index should always be valid,
				// as we've checked it's a leaf node already but this is to be safe
				// maybe the code should be strong enough can this assert isn't necessary
				assert(childIndex < OctTree::TOTAL_NUMBER_OF_NODES && "Out of OctTree allNodes bounds");

				OctTreeNode& child = allNodes[childIndex];
				// now check if the entity can be inserted into any of the child nodes
				bool entityFitsInsideNode = DoesEntityFitInNode(
					entityAABB,
					child.GetAABB()
				);

				if (entityFitsInsideNode) {
					// if it can fit in the child node, we insert it there and break out of the loop
					return Insert_Internal(entityIndex, entityAABB, isStatic, child, depth + 1);
				}
			}
		}

		// 3. if we're at a leaf node OR
		// none of the children could completely insert it even though 
		// it's small enough, add it to (parent) node's entity list
		if (isStatic) {
			node.UpdateStaticEntities(entityIndex);
		}
		else {
			node.UpdateDynamicEntities(entityIndex);
		}

		return true;
	}

	void OctTree::Subdivide(OctTreeNode& node, float childrenHalfWidth) {
		uint32_t startIndex = mNextAvailableStartIndex;
		node.SetStartIndexOfChildNodes(mNextAvailableStartIndex);
		IncrementNextAvailableStartIndex();

		for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
			// node index = startIndex + i
			uint32_t childNodeIndex = startIndex + i;
			if (childNodeIndex >= OctTree::TOTAL_NUMBER_OF_NODES) { break; }

			OctTreeNode& childNode = allNodes[childNodeIndex];

			// calculate center based on the index
			// 
			// first 3 bits of 'i' are implicity mapped to directions x, y, z,
			// so that we can easily find the new x/y/z of child node based on 
			// if we have to go in the positive or negative direction 
			// from (parent) node's center. Basically every child node has 
			// a fixed index 0 to 7 and  at each 'i' we are using bit masking 
			// to map which way  the new center is for this index from parent center
			// for example if we have index 5 (101 in binary) we will go positive x, negative y and positive z

			Vector3 center = node.GetCenter();
			// we check the first bit, if it's a 1 we go positive x, else negative x
			center.x += ((i & 1) ? childrenHalfWidth : -childrenHalfWidth);
			// we check the second bit, if it's a 1 we go positive y, else negative y
			center.y += ((i & 2) ? childrenHalfWidth : -childrenHalfWidth);
			// we check the third bit, if it's a 1 we go positive z, else negative z
			center.z += ((i & 4) ? childrenHalfWidth : -childrenHalfWidth);

			childNode.SetCenter(center);
			childNode.SetHalfWidth(childrenHalfWidth);
			CalculateAndUpdateBoundsOfNode(childNode);
		}
	}

	void OctTree::CalculateAndUpdateBoundsOfNode(OctTreeNode& node) {
		const Vector3& center = node.GetCenter();
		float halfWidth = node.GetHalfWidth();

		Vector3 nodeMin = center - halfWidth;
		Vector3 nodeMax = center + halfWidth;

		node.SetBounds(nodeMin, nodeMax);
	}

	bool OctTree::DoesEntityFitInNode(
		const AABB& entityAABB, 
		const AABB& nodeAABB
	) const {
		return
			// nodeMin has to be less than or equal to entity min
			(nodeAABB.Min.x <= entityAABB.Min.x) &&
			(nodeAABB.Min.y <= entityAABB.Min.y) &&
			(nodeAABB.Min.z <= entityAABB.Min.z) &&
			// nodeMax has to be greater than or equal to entity max
			(nodeAABB.Max.x >= entityAABB.Max.x) &&
			(nodeAABB.Max.y >= entityAABB.Max.y) &&
			(nodeAABB.Max.z >= entityAABB.Max.z);
	}

	void OctTree::GetPotentalCollisionsWithPlayer_Internal(
		uint32_t startIndex,
		const AABB& playerPotentialAABB,
		std::vector<uint32_t>& potentialCandidates
	) {
		if (startIndex >= mNextAvailableStartIndex) { return; }

		OctTreeNode& node = allNodes[startIndex];

		// if player doesn't intersect with this node, return
		if (!GeometryHelper::AABBIntersect(playerPotentialAABB, node.GetAABB())) { return; }

		// add all entities inside this node to candidates
		for (uint32_t entityIndex : node.GetStaticEntities()) {
			potentialCandidates.push_back(entityIndex);
		}
		for (uint32_t entityIndex : node.GetDynamicEntities()) {
			potentialCandidates.push_back(entityIndex);
		}

		// recurse through it's children, if it has any
		if (node.GetStartIndexOfChildren() == OctTreeNode::INVALID_START_INDEX) {
			return;
		}

		for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
			GetPotentalCollisionsWithPlayer_Internal(
				node.GetStartIndexOfChildren() + i,
				playerPotentialAABB,
				potentialCandidates
			);
		}
	}
}

#pragma endregion