#include "Mesh.h"

namespace Engine {

	Mesh::Mesh() :
		mMeshID(MeshID::Count),
		mVbByteSize(0),
		mIbByteSize(0)
	{}

	Mesh::~Mesh() {}

	void Mesh::Load(
		MeshID id,
		std::vector<Vertex> vertices, 
		std::vector<uint16_t> indices
	) {
		mMeshID = id;
		mVertices = vertices;
		mIndices = indices;

		mVbByteSize = (uint32_t)vertices.size() * sizeof(Vertex);
		mIbByteSize = (uint32_t)indices.size() * sizeof(uint16_t);

		CalculateLocalMinAndMax();
	}

	const std::vector<Vertex>& Mesh::GetVertices() const {
		return mVertices;
	}

	const std::vector<uint16_t>& Mesh::GetIndices() const {
		return mIndices;
	}

	MeshID Mesh::GetMeshID() const { return mMeshID; }

	UINT Mesh::GetVbByteSize() const { return mVbByteSize; }

	UINT Mesh::GetIbByteSize() const { return mIbByteSize; }

	const Vector3& Mesh::GetLocalMin() const { return mLocalMin; }

	const Vector3& Mesh::GetLocalMax() const { return mLocalMax; }

	void Mesh::CalculateLocalMinAndMax() {
		if (mVertices.size() < 1) { return; }

		Vector3 min = mVertices[0].Position;
		Vector3 max = mVertices[0].Position;

		for (int i = 1; i < mVertices.size(); ++i) {

			const auto& pos = mVertices[i].Position;
			// min and max x
			if (pos.x < min.x) { min.x = pos.x; }
			if (pos.x > max.x) { max.x = pos.x; }

			// min and max y
			if (pos.y < min.y) { min.y = pos.y; }
			if (pos.y > max.y) { max.y = pos.y; }

			// min and max z
			if (pos.z < min.z) { min.z = pos.z; }
			if (pos.z > max.z) { max.z = pos.z; }
		}

		mLocalMin = min;
		mLocalMax = max;
	}
}