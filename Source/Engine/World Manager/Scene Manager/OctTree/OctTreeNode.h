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
		const std::vector<const Entity*>& GetStaticEntities() const;
		const std::vector<const Entity*>& GetDynamicEntities() const;

		void SetCenter(const Vector3& center);
		void SetHalfWidth(float halfWidth);
		void SetBounds(Vector3 min, Vector3 max);
		void SetStartIndexOfChildNodes(uint32_t startingIndex);

		void UpdateStaticEntities(const Entity* entity);
		void UpdateDynamicEntities(const Entity* entity);
		void ClearDynamicEntities();

	public:
		static constexpr uint32_t INVALID_START_INDEX = UINT32_MAX;
		static constexpr uint32_t NUMBER_OF_CHILDREN = 8;

	private:
		Vector3 mCenter;
		AABB mAABB;
		float mHalfWidth;
		std::vector<const Entity*> mStaticEntities;
		std::vector<const Entity*> mDynamicEntities;
		uint32_t mStartIndexOfChildNodes;
	};
}