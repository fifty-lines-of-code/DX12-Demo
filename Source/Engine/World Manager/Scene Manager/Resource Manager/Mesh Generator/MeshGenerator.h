#pragma once

#include <cstdint>
#include "../../../../Math/Geometry.h"
#include "Mesh/Mesh.h"
#include <vector>

namespace Engine::EngineResources {

	class MeshGenerator {
	public:
		MeshGenerator() = default;
		~MeshGenerator() = default;

		MeshGenerator(const MeshGenerator&) = delete;
		MeshGenerator& operator=(const MeshGenerator&) = delete;

		void GenerateCubeMesh(
			std::vector<Vertex>& outVertices,
			std::vector<uint16_t>& outIndices,
			std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH>& subMeshes
		) const noexcept;

		void GenerateMirrorMesh(
			std::vector<Vertex>& outVertices,
			std::vector<uint16_t>& outIndices,
			std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH>& subMeshes
		) const noexcept;

	private:
	};
}