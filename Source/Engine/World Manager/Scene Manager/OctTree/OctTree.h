#pragma once

#include <cmath>
#include "../../../Math/EngineMath.h"
#include "OctTreeNode.h"

class Entity;
struct AABB;

namespace Engine {
	
	class OctTree {
	public:
		OctTree();
		~OctTree();

		bool Initialize(const Vector3& center, float halfWidth);
		bool Insert(uint32_t entityIndex, const AABB& entityAABB, bool isStatic);
		void ClearDynamicEntities();
		void GetPotentialCollisionsWithAABB(uint32_t playerID, const AABB& playerPotentialAABB, std::vector<uint32_t>& candidates);

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
		// credit: Google Gemini
		static constexpr uint32_t TOTAL_NUMBER_OF_NODES =
			((1 << (3 * (MAX_DEPTH + 1))) - 1) / 7;

		OctTreeNode& mRoot;
		OctTreeNode allNodes[OctTree::TOTAL_NUMBER_OF_NODES];
		uint32_t mNextAvailableStartIndex = 1; // index 0 will store root

	private:
		void SetupRoot(const Vector3& center, float halfWidth);
		void IncrementNextAvailableStartIndex();
		bool Insert_Internal(uint32_t entityIndex, const AABB& entityAABB, bool isStatic, OctTreeNode& node, uint32_t depth);
		void Subdivide(OctTreeNode& node, float childrenHalfWidth);
		void CalculateAndUpdateBoundsOfNode(OctTreeNode& node);
		bool DoesEntityFitInNode(const AABB& entityAABB, const AABB& nodeAABB) const;

		void GetPotentalCollisionsWithPlayer_Internal(uint32_t startIndex, const AABB& playerPotentialAABB, std::vector<uint32_t>& potentialCandidates);
	};
}