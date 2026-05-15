#include "OctTree.h"

#include "../Entities/Entity.h"

OctTreeNode::~OctTreeNode() = default;

OctTree::OctTree(const DirectX::XMFLOAT3 center, float halfWidth) {
	mRoot = std::make_unique<OctTreeNode>(center, halfWidth);
	CalculateMinAndMax(mRoot.get());
}

OctTree::~OctTree() {
}

bool OctTree::Insert(const Entity* entity) {
	// early exit if no root for some reason
	if (mRoot == nullptr) {
		return false;
	}

	return Insert_Internal(entity, mRoot.get(), 0);
}

bool OctTree::Insert_Internal(const Entity* entity, OctTreeNode* const node, uint32_t depth) {
	// 0. if this is the deepest we can go, store in node's entities
	if (depth >= OctTree::MAX_DEPTH) {
		node->UpdateEntities(entity);
		return true;
	}

	EntityAABBMinMax entityAABBMinMax = entity->GetAABBMinMax();

	// 1. if entity is larger than child nodes, store it in this node
	// (can combine steps 0 and 1 into a single if, but i am choosing clarity)
	double childrenHalfWidth = node->GetHalfWidth() * 0.5;
	if (childrenHalfWidth < entityAABBMinMax.halfWidth) {
		// children's half width is smaller than entity's half width, store in root
		node->UpdateEntities(entity);
		return true;
	}

	// 2. Subdivide the node if it doesn't have children already
	// assumed if first is nullptr, we need to subdivide the node
	// and conversely if first is not nullptr, we already have children
	// and we can skip subdivison
	if (node->GetChildAtIndex(0) == nullptr) {
		Subdivide(node, (float)childrenHalfWidth);
	}

	// 3. Find the child node to insert into
	for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
		OctTreeNode* child = node->GetChildAtIndex(i);
		// now check if the entity can be inserted into any of the child nodes
		if (DoesEntityFitInNode(entityAABBMinMax, child->GetHalfWidth(), child->GetCenter(), child->GetMin(), child->GetMax())) {
			// if it can fit in the child node, we insert it there and break out of the loop
			return Insert_Internal(entity, child, depth + 1);
		}
	}

	// 4. if none of the children could completely insert it even though it's small enough
	// add it to (parent) node's entity list
	node->UpdateEntities(entity);

	return true;
}

void OctTree::Subdivide(OctTreeNode* node, float childrenHalfWidth) {
	for (int i = 0; i < OctTreeNode::NUMBER_OF_CHILDREN; ++i) {
		// calculate center based on the index

		// first 3 bits of 'i' are implicity mapped to directions x, y, z, so that we can 
		// easily find the new x/y/z of child node based on if we have to go in the positive or negative direction from (parent) node's center.
		// basically every child node has a fixed index 0 to 7 and 
		// at each 'i' we are using bit masking to map which way 
		// the new center is for this index from parent center
		// for example if we have index 5 (101 in binary) we will go positive x, negative y and positive z

		DirectX::XMFLOAT3 center = node->GetCenter();
		// we check the first bit, if it's a 1 we go positive x, else negative x
		center.x += ((i & 1) ? childrenHalfWidth : -childrenHalfWidth);
		// we check the second bit, if it's a 1 we go positive y, else negative y
		center.y += ((i & 2) ? childrenHalfWidth : -childrenHalfWidth);
		// we check the third bit, if it's a 1 we go positive z, else negative z
		center.z += ((i & 4) ? childrenHalfWidth : -childrenHalfWidth);

		std::unique_ptr<OctTreeNode> childNode = std::make_unique<OctTreeNode>(center, childrenHalfWidth);
		CalculateMinAndMax(childNode.get());
		node->UpdateChildAtIndex(i, std::move(childNode));
	}
}

bool OctTree::DoesEntityFitInNode(EntityAABBMinMax entityAABB, float nodeHalfSize, DirectX::XMFLOAT3 nodeCenter, DirectX::XMFLOAT3 nodeMin, DirectX::XMFLOAT3 nodeMax) {
	// todo

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

void OctTree::CalculateMinAndMax(OctTreeNode* node) {
	DirectX::XMFLOAT3 center = node->GetCenter();
	DirectX::XMFLOAT3 nodeMin = center;
	float halfWidth = node->GetHalfWidth();
	nodeMin.x = center.x - halfWidth;
	nodeMin.y = center.y - halfWidth;
	nodeMin.z = center.z - halfWidth;
	DirectX::XMFLOAT3 nodeMax = center;
	nodeMax.x = center.x + halfWidth;
	nodeMax.y = center.y + halfWidth;
	nodeMax.z = center.z + halfWidth;

	node->SetBounds(nodeMin, nodeMax);
}
