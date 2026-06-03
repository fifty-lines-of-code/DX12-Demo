#include "TerrainManager.h"

namespace Engine {

	TerrainManager::TerrainManager(uint16_t chunkSize) : 
		mChunkSize(chunkSize),
		mChunkSizeHalf(chunkSize/2),
		mNumberOfVerticesPerEdge(chunkSize + 1)
	{}

	TerrainManager::~TerrainManager() {}

	uint16_t TerrainManager::TotalNumberOfVerticesForChunk() const noexcept {
		return (mChunkSize + 1) * (mChunkSize + 1);
	}

	void TerrainManager::GenerateTerrainFor(
		float chunkCenterX, 
		float chunkCenterZ, 
		std::vector<Vertex>& outVertices, 
		std::vector<uint16_t>& outIndices
	) {
		float startX = chunkCenterX - mChunkSizeHalf;
		float startZ = chunkCenterZ - mChunkSizeHalf;

		uint16_t indexPtr = 0;
		Vertex v;

		for (int z = 0; z < mNumberOfVerticesPerEdge; ++z) {
			for (int x = 0; x < mNumberOfVerticesPerEdge; ++x) {
				uint16_t vIndex = (z * mNumberOfVerticesPerEdge) + x;

				v.Position.x = startX + static_cast<float>(x);
				// generate a flat terrian, then add y in a different pass
				v.Position.y = 0;
				v.Position.z = startZ + static_cast<float>(z);

				// default color for now
				// during y pass maybe we can add color
				// or a completely different pass
				v.Color = Vector4(0.25, 0.25, 0.25, 1.0);

				outVertices.push_back(v);

				// build 2 traingles looking forward, from the quad formed 
				// and this vertex is the topLeft of that quad
				if (z < mNumberOfVerticesPerEdge - 1 && x < mNumberOfVerticesPerEdge - 1) {
					uint16_t topLeft = vIndex + mNumberOfVerticesPerEdge;
					uint16_t topRight = topLeft + 1;
					uint16_t bottomLeft = vIndex;
					uint16_t bottomRight = bottomLeft + 1;

					// triangle 1
					// top left, top right, bottom left
					outIndices.push_back(bottomLeft);
					outIndices.push_back(topLeft);
					outIndices.push_back(topRight);

					// triangle 2
					// bottom left, top right, bottom right
					outIndices.push_back(bottomLeft);
					outIndices.push_back(topRight);
					outIndices.push_back(bottomRight);
				}
			}
		}
	}
}