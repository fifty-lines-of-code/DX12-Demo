#pragma once

#include <array>
#include "../../../../../EngineConfig.h"
#include "../../../../../Math/Geometry.h"
#include <vector>
#include <wrl.h>

namespace Engine::EngineResources {

	enum class MeshID : uint32_t {
		CUBE = 0,
		MIRROR,
		TERRAIN_0x0,
		COUNT,
		INVALID
	};

	struct SubMesh {
		uint32_t IndexCount = 0;
		uint32_t StartIndexLocation = 0;
		uint32_t BaseVertexLocation = 0;
	};

	class Mesh {
	public:
		Mesh();
		~Mesh();

		void Load(
			MeshID id,
			std::vector<Vertex> vertices,
			std::vector<uint16_t> indices,
			std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> submeshes,
			uint8_t activeSubMeshCount
		) noexcept;

		const std::vector<Vertex>& GetVertices() const noexcept;
		const std::vector<uint16_t>& GetIndices() const noexcept;
		MeshID GetMeshID() const noexcept;
		UINT GetVbByteSize() const noexcept;
		UINT GetIbByteSize() const noexcept;
		const Vector3& GetLocalMin() const noexcept;
		const Vector3& GetLocalMax() const noexcept;
		uint8_t GetActiveSubMeshCount() const noexcept;
		const SubMesh& GetSubMeshAtIndex(uint8_t index) const noexcept;

		bool GetIsReadyToLoad() const noexcept;

	private:
		std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> mSubMeshes;
		// todo: extract below into flat arrays of max size
		// instead of vectors
		std::vector<Vertex> mVertices;
		std::vector<uint16_t> mIndices;

		Vector3 mLocalMin;
		Vector3 mLocalMax;
		MeshID mMeshID;
		uint32_t mVbByteSize;
		uint32_t mIbByteSize;
		uint8_t mActiveSubMeshCount;
		bool mIsReadyToLoad;

	private:
		void CalculateLocalMinAndMax();
	};
}