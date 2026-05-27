#pragma once

#include "../../../Math/EngineMath.h"
#include <memory>
#include <vector>

class Entity;

namespace Engine {

	struct OctTreeNode {
	public:
		OctTreeNode();
		OctTreeNode(Vector3 center, float halfWidth);
		~OctTreeNode();

		const Vector3& GetCenter() const;
		float GetHalfWidth() const;
		const Vector3& GetMin() const;
		const Vector3& GetMax() const;
		uint32_t GetStartIndexOfChildNodes() const;

		void SetCenter(const Vector3& center);
		void SetHalfWidth(float halfWidth);
		void SetBounds(Vector3 min, Vector3 max);
		void SetStartIndexOfChildNodes(uint32_t startingIndex);
		void UpdateEntities(const Entity* entity);

		static constexpr uint32_t NUMBER_OF_CHILDREN = 8;

	private:
		Vector3 mCenter;
		Vector3 mNodeMin;
		Vector3 mNodeMax;
		float mHalfWidth;
		std::vector<const Entity*> mEntities;
		uint32_t mStartIndexOfChildNodes;
	};
}