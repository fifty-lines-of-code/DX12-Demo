#pragma once

#include <cmath>
#include "../../../Math/EngineMath.h"
#include "OctTreeNode.h"

class Entity;
struct AABB;

namespace Engine {
	
	class OctTree {
	public:
		OctTree(const Vector3 center, float halfWidth);
		~OctTree();
		bool Insert(const Entity* entity);

	private:
		// why depth of 3? Because our scene is small and we don't have many entities, so we don't need a very deep tree, but helps to learn the idea
		// in the real world it's usually between 5 and 8, but it really depends on the scene and the number of entities (or so copiolot says)
		// Remember: this created 4 layers
		static constexpr uint32_t MAX_DEPTH = 3;

		// 8 is 2^3, which is left shifting 1 by 3 places
		// so if we want 8 ^ (x), we left shift 1 by (3 * x) times
		// i.e. 1 << (3 * x)
		// 8 ^ 3 = 512
		// 1 << (3 * 3) = 512
		// credit: Google Gemini

		// the formula comes from Geometric Series Sum Formula
		// create: Google Gemini
		static constexpr uint32_t TOTAL_NUMBER_OF_NODES =
			((1 << (3 * (MAX_DEPTH + 1))) - 1) / 7;

		OctTreeNode* mRoot;
		OctTreeNode allNodes[OctTree::TOTAL_NUMBER_OF_NODES];
		uint32_t mNextAvailableStartIndex;

	private:
		void SetupRoot(float halfWidth);
		void IncrementNextAvailableStartIndex();
		bool Insert_Internal(const Entity* entity, OctTreeNode* const node, uint32_t depth);
		void Subdivide(OctTreeNode* const node, float childrenHalfWidth);
		void CalculateAndUpdateBoundsOfNode(OctTreeNode* node);
		bool DoesEntityFitInNode(const AABB& entityAABB, const Vector3& nodeMin, const Vector3& nodeMax) const;
	};
}