#pragma once

#include "../../../../Math/Geometry.h"
#include <vector>

namespace Engine {

	enum class TerrainLOD {
		LOW,
		MED,
		HIGH
	};

	class TerrainManager {
	public:
		TerrainManager(uint16_t chunkSize);
		~TerrainManager();

		void GenerateTerrainFor(
			float chunkCenterX,
			float chunkCenterZ, 
			std::vector<Vertex>& vertices, 
			std::vector<uint16_t>& indices
		);

		uint16_t TotalNumberOfVerticesForChunk() const noexcept;

	private:
		uint16_t mChunkSize;
		uint16_t mChunkSizeHalf;
		const uint16_t mNumberOfVerticesPerEdge;
	};
}