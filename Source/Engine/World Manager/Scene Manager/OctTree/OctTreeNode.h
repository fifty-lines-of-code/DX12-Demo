#pragma once

#include "../Entity/Entity.h"
#include "../../../Math/EngineMath.h"
#include <memory>
#include <vector>

namespace Engine {

	struct OctTreeNode {
	public:
		OctTreeNode();
		OctTreeNode(Vector3 center, float halfWidth);
		~OctTreeNode();

		const Vector3& GetCenter() const;
		float GetHalfWidth() const;
		const AABB& GetAABB() const;
		uint32_t GetStartIndexOfChildren() const;
		const std::vector<uint32_t>& GetStaticEntities() const;
		const std::vector<uint32_t>& GetDynamicEntities() const;

		void SetCenter(const Vector3& center);
		void SetHalfWidth(float halfWidth);
		void SetBounds(Vector3 min, Vector3 max);
		void SetStartIndexOfChildNodes(uint32_t startingIndex);

		void UpdateStaticEntities(uint32_t entityIndex);
		void UpdateDynamicEntities(uint32_t entityIndex);
		void ClearDynamicEntities();

	public:
		static constexpr uint32_t INVALID_START_INDEX = UINT32_MAX;
		static constexpr uint32_t NUMBER_OF_CHILDREN = 8;

	private:
		// we store the index of the entities 
		std::vector<uint32_t> mStaticEntities;
		std::vector<uint32_t> mDynamicEntities;
		AABB mAABB;
		Vector3 mCenter;
		float mHalfWidth;
		uint32_t mStartIndexOfChildNodes;
	};
}