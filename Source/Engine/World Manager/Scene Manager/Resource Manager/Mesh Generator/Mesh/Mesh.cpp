#include "Mesh.h"

#include "../../../../../../Helper/Logger.h"

namespace Engine::EngineResources {

	Mesh::Mesh() :
		mMeshID(MeshID::COUNT),
		mVbByteSize(0),
		mIbByteSize(0),
		mActiveSubMeshCount(0)
	{}

	Mesh::~Mesh() {}

	void Mesh::Load(
		MeshID id,
		std::vector<Vertex> vertices, 
		std::vector<uint16_t> indices,
		std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> submeshes,
		uint8_t activeSubMeshCount
	) noexcept
	{
		mMeshID = id;
		mVertices = vertices;
		mIndices = indices;
		mSubMeshes = submeshes;
		mActiveSubMeshCount = activeSubMeshCount;

		mVbByteSize = (uint32_t)vertices.size() * sizeof(Vertex);
		mIbByteSize = (uint32_t)indices.size() * sizeof(uint16_t);

		CalculateLocalMinAndMax();
	}

	const std::vector<Vertex>& Mesh::GetVertices() const noexcept {
		return mVertices;
	}

	const std::vector<uint16_t>& Mesh::GetIndices() const noexcept  {
		return mIndices;
	}

	MeshID Mesh::GetMeshID() const noexcept  { return mMeshID; }

	UINT Mesh::GetVbByteSize() const noexcept { return mVbByteSize; }

	UINT Mesh::GetIbByteSize() const noexcept  { return mIbByteSize; }

	const Vector3& Mesh::GetLocalMin() const noexcept { return mLocalMin; }

	const Vector3& Mesh::GetLocalMax() const noexcept { return mLocalMax; }

	uint8_t Mesh::GetActiveSubMeshCount() const noexcept { return mActiveSubMeshCount; }

	const SubMesh& Mesh::GetSubMeshAtIndex(uint8_t index) const noexcept {
		if (index >= EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH) {
			Logger::ERR(L"SubMesh index is incorrect, expect errors!");
			return mSubMeshes[0];
		}

		return mSubMeshes[index];
	}

#pragma region Private

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

#pragma endregion
}