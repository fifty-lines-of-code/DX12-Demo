#pragma once

#include "../../../../Math/Geometry.h"
#include <vector>
#include <wrl.h>

namespace Engine {

	enum class MeshID : uint32_t {
		Cube = 0,
		Terrain0x0,
		Count
	};

	class Mesh {
	public:
		Mesh();
		~Mesh();

		void Load(
			MeshID id,
			std::vector<Vertex> vertices,
			std::vector<uint16_t> indices
		);

		const std::vector<Vertex>& GetVertices() const;
		const std::vector<uint16_t>& GetIndices() const;
		MeshID GetMeshID() const;
		UINT GetVbByteSize() const;
		UINT GetIbByteSize() const;
		const Vector3& GetLocalMin() const;
		const Vector3& GetLocalMax() const;

	private:
		std::vector<Vertex> mVertices;
		std::vector<uint16_t> mIndices;

		MeshID mMeshID;
		uint32_t mVbByteSize;
		uint32_t mIbByteSize;
		Vector3 mLocalMin;
		Vector3 mLocalMax;

	private:
		void CalculateLocalMinAndMax();
	};
}