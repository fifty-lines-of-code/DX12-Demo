#pragma once

#include "../../../../Math/Geometry.h"
#include <string>
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
			TerrainLOD lod,
			std::vector<Vertex>& vertices, 
			std::vector<uint16_t>& indices
		);

		uint16_t TotalNumberOfVerticesForChunk(TerrainLOD lod) const noexcept;
		uint32_t TotalNumberOfIndicesForChunk(TerrainLOD lod) const noexcept;

	private:
		const float mMaxHeight = 3.f;
		uint16_t mChunkSize;
		uint16_t mChunkSizeHalf;

	private:
		bool LoadHeightmapToArray(
			const std::string& filename,
			std::array<uint8_t, 1089>& outPixelArray
		);
	};
}