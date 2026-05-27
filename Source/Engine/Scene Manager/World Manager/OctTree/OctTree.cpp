#include "OctTree.h"

#include "../../Entities/Entity.h"

namespace Engine {

	OctTree::OctTree(Vector3 center, float halfWidth) :
		mRoot(nullptr),
		mNextAvailableStartIndex(1)
	{
		SetupRoot(halfWidth);
		IncrementNextAvailableStartIndex();
	}

	OctTree::~OctTree() {}

	bool OctTree::Insert(const Entity* entity) {
		return Insert_Internal(entity, mRoot, 0);
	}

#pragma region Private

	void OctTree::SetupRoot(float halfWidth) {
		mRoot = &allNodes[0];
		mRoot->SetHalfWidth(halfWidth);
		mRoot->SetStartIndexOfChildNodes(mNextAvailableStartIndex);
		CalculateAndUpdateBoundsOfNode(mRoot);
	}

	void OctTree::IncrementNextAvailableStartIndex() {
		mNextAvailableStartIndex += OctTreeNode::NUMBER_OF_CHILDREN;
	}

	bool OctTree::Insert_Internal(const Entity* entity, OctTreeNode* const node, uint32_t depth) {
		if (node == nullptr) { return false; }

		// if this isn't the deepest level (leaf nodes)
		if (depth < OctTree::MAX_DEPTH) {
			AABB entityAABB = entity->GetAABB();
			double childrenHalfWidth = node->GetHalfWidth() * 0.5;

			// 1. Subdivide the node if it doesn't have children already
			// known by if start index == uint32_t::max
			// maybe we can find a better way to know if a node has been subdivided
			if (node->GetStartIndexOfChildNodes() == UINT32_MAX) {
				Subdivide(node, (float)childrenHalfWidth);
			}

			uint32_t startIndex = node->GetStartIndexOfChildNodes();
			// 2. Find the child node to insert into
			for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
				uint32_t childIndex = startIndex + i;

				// theoretically index should always be valid,
				// as we've checked it's a leaf node already but this is to be safe
				// maybe the code should be strong enough can this assert isn't necessary
				assert(childIndex < OctTree::TOTAL_NUMBER_OF_NODES && "Out of OctTree allNodes bounds");

				OctTreeNode* const child = &allNodes[childIndex];
				// now check if the entity can be inserted into any of the child nodes
				bool entityFitsInsideNode = DoesEntityFitInNode(
					entityAABB,
					child->GetMin(),
					child->GetMax()
				);

				if (entityFitsInsideNode) {
					// if it can fit in the child node, we insert it there and break out of the loop
					return Insert_Internal(entity, child, depth + 1);
				}
			}
		}

		// 3. if none of the children could completely insert it even though 
		// it's small enough add it to (parent) node's entity list
		node->UpdateEntities(entity);

		return true;
	}

	void OctTree::Subdivide(OctTreeNode* const node, float childrenHalfWidth) {
		uint32_t startIndex = mNextAvailableStartIndex;
		node->SetStartIndexOfChildNodes(mNextAvailableStartIndex);
		IncrementNextAvailableStartIndex();

		for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
			// node index = startIndex + i
			uint32_t childNodeIndex = startIndex + i;
			if (childNodeIndex >= OctTree::TOTAL_NUMBER_OF_NODES) { break; }

			OctTreeNode* childNode = &allNodes[childNodeIndex];

			// calculate center based on the index
			// 
			// first 3 bits of 'i' are implicity mapped to directions x, y, z,
			// so that we can easily find the new x/y/z of child node based on 
			// if we have to go in the positive or negative direction 
			// from (parent) node's center. Basically every child node has 
			// a fixed index 0 to 7 and  at each 'i' we are using bit masking 
			// to map which way  the new center is for this index from parent center
			// for example if we have index 5 (101 in binary) we will go positive x, negative y and positive z

			Vector3 center = node->GetCenter();
			// we check the first bit, if it's a 1 we go positive x, else negative x
			center.x += ((i & 1) ? childrenHalfWidth : -childrenHalfWidth);
			// we check the second bit, if it's a 1 we go positive y, else negative y
			center.y += ((i & 2) ? childrenHalfWidth : -childrenHalfWidth);
			// we check the third bit, if it's a 1 we go positive z, else negative z
			center.z += ((i & 4) ? childrenHalfWidth : -childrenHalfWidth);

			childNode->SetCenter(center);
			childNode->SetHalfWidth(childrenHalfWidth);
			CalculateAndUpdateBoundsOfNode(childNode);
		}
	}

	void OctTree::CalculateAndUpdateBoundsOfNode(OctTreeNode* node) {
		const Vector3& center = node->GetCenter();
		float halfWidth = node->GetHalfWidth();

		Vector3 nodeMin = center - halfWidth;
		Vector3 nodeMax = center + halfWidth;

		node->SetBounds(nodeMin, nodeMax);
	}

	bool OctTree::DoesEntityFitInNode(
		const AABB& entityAABB, 
		const Vector3& nodeMin, 
		const Vector3& nodeMax
	) const {
		return
			// nodeMin has to be less than or equal to entity min
			(nodeMin.x <= entityAABB.Min.x) &&
			(nodeMin.y <= entityAABB.Min.y) &&
			(nodeMin.z <= entityAABB.Min.z) &&
			// nodeMax has to be greater than or equal to entity max
			(nodeMax.x >= entityAABB.Max.x) &&
			(nodeMax.y >= entityAABB.Max.y) &&
			(nodeMax.z >= entityAABB.Max.z);
	}
}

#pragma endregion