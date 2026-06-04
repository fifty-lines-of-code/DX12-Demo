#include "TerrainManager.h"

#include <array>
#include <cmath>
#include "../../../../../Helper/Logger.h"
#include "../../../../Math/ColorHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../../../Helper/stb_image.h"

namespace Engine {

	TerrainManager::TerrainManager(uint16_t chunkSize) :
		mChunkSize(chunkSize),
		mChunkSizeHalf(chunkSize / 2),
		mNumberOfVerticesPerEdge(chunkSize + 1)
	{
		stbi_set_flip_vertically_on_load(true);
	}

	TerrainManager::~TerrainManager() {}

	uint16_t TerrainManager::TotalNumberOfVerticesForChunk() const noexcept {
		return (mChunkSize + 1) * (mChunkSize + 1);
	}

	void TerrainManager::GenerateTerrainFor(
		float chunkCenterX,
		float chunkCenterZ,
		TerrainLOD lod,
		std::vector<Vertex>& outVertices,
		std::vector<uint16_t>& outIndices
	) {
		float startX = chunkCenterX - mChunkSizeHalf;
		float startZ = chunkCenterZ - mChunkSizeHalf;

		std::array<uint8_t, 1089> heightValues{};

		if (!LoadHeightmapToArray("Source/Resources/Heightmaps/Chunk0x0-1.png", heightValues)) {
			assert(false && "Terrain texture asset could not be processed.");
		}

		// 1. Determine our vertex subdivision factor based on LOD
		// lod == Low    -> 33x33 Vertices  (original setup)
		// lod == Medium -> 65x65 Vertices  (Double triangle density)
		// lod == High   -> 129x129 Vertices (Quadruple triangle density)
		int densityMultiplier = 1;
		if (lod == TerrainLOD::MED)     densityMultiplier = 2;
		else if (lod == TerrainLOD::HIGH)  densityMultiplier = 4;

		// 2. Calculate vertex configurations based on LOD
		int vertsPerEdgeLOD = ((mNumberOfVerticesPerEdge - 1) * densityMultiplier) + 1;

		// Perfect world-space spacing to ensure the physical bounds stay locked at mChunkSize (32)
		float vertexSpacing = static_cast<float>(mChunkSize) / static_cast<float>(vertsPerEdgeLOD - 1);

		// How far we advance across our 33x33 source image per vertex index loop step
		float imageSampleStride = 1.0f / static_cast<float>(densityMultiplier);

		Vertex v;

		// 3. Loop over our dynamic vertex density limits
		for (int z = 0; z < vertsPerEdgeLOD; ++z) {
			for (int x = 0; x < vertsPerEdgeLOD; ++x) {

				// Set absolute positions stretched cleanly across the chunk size
				v.Position.x = startX + (static_cast<float>(x) * vertexSpacing);
				v.Position.z = startZ + (static_cast<float>(z) * vertexSpacing);

				// 4. BILINEAR HEIGHT INTERPOLATION
				// Map the current loop index back into fractional floating image coordinates
				float imgX = static_cast<float>(x) * imageSampleStride;
				float imgZ = static_cast<float>(z) * imageSampleStride;

				// Identify neighboring pixels enclosing our fractional point
				int x0 = static_cast<int>(std::floor(imgX));
				int x1 = (x0 < mNumberOfVerticesPerEdge - 1) ? x0 + 1 : x0;
				int z0 = static_cast<int>(std::floor(imgZ));
				int z1 = (z0 < mNumberOfVerticesPerEdge - 1) ? z0 + 1 : z0;

				// Calculate interpolation weights (0.0 to 1.0 distances)
				float tx = imgX - static_cast<float>(x0);
				float tz = imgZ - static_cast<float>(z0);

				// Sample 4 height corners from your 33x33 pixel buffer
				float h00 = static_cast<float>(heightValues[(z0 * mNumberOfVerticesPerEdge) + x0]);
				float h10 = static_cast<float>(heightValues[(z0 * mNumberOfVerticesPerEdge) + x1]);
				float h01 = static_cast<float>(heightValues[(z1 * mNumberOfVerticesPerEdge) + x0]);
				float h11 = static_cast<float>(heightValues[(z1 * mNumberOfVerticesPerEdge) + x1]);

				// Blend the heights horizontally, then combine vertically
				float h0 = h00 + tx * (h10 - h00);
				float h1 = h01 + tx * (h11 - h01);
				float finalSampledHeight = h0 + tz * (h1 - h0);

				float normalizedHeight = finalSampledHeight / 55.f;
				if (normalizedHeight > 1.f) { normalizedHeight = 1.f; }
				v.Position.y = normalizedHeight * mMaxHeight;

				// Shading Pipeline
				Vector4 finalColor;
				if (normalizedHeight < 0.2f) {
					float t = normalizedHeight / 0.2f;
					finalColor = ColorHelper::LerpColor(ColorHelper::ColorValley, ColorHelper::ColorGrass, t);
					finalColor = ColorHelper::LerpColor(ColorHelper::ColorValley, ColorHelper::ColorGrass, t);
				}
				else if (normalizedHeight < 0.6f) {
					float t = (normalizedHeight - 0.2f) / (0.6f - 0.2f);
					finalColor = ColorHelper::LerpColor(ColorHelper::ColorGrass, ColorHelper::ColorRock, t);
				}
				else {
					float t = (normalizedHeight - 0.6f) / (1.0f - 0.6f);
					finalColor = ColorHelper::LerpColor(ColorHelper::ColorRock, ColorHelper::ColorSnow, t);
				}
				v.Color = finalColor;

				outVertices.push_back(v);

				// 5. Build index buffer using the structural constraints of the high density mesh limits
				if (z < vertsPerEdgeLOD - 1 && x < vertsPerEdgeLOD - 1) {
					uint16_t lodIndex = (z * vertsPerEdgeLOD) + x;

					uint16_t bottomLeft = lodIndex;
					uint16_t bottomRight = bottomLeft + 1;
					uint16_t topLeft = lodIndex + vertsPerEdgeLOD;
					uint16_t topRight = topLeft + 1;

					// Triangle 1
					outIndices.push_back(bottomLeft);
					outIndices.push_back(topLeft);
					outIndices.push_back(topRight);

					// Triangle 2
					outIndices.push_back(bottomLeft);
					outIndices.push_back(topRight);
					outIndices.push_back(bottomRight);
				}
			}
		}
	}

#pragma region Private
	bool TerrainManager::LoadHeightmapToArray(
		const std::string& filename,
		std::array<uint8_t, 1089>& outPixelArray
	) {
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channelsInFile = 0;

		uint8_t* rawData = stbi_load(filename.c_str(), &width, &height, &channelsInFile, 1);

		if (!rawData) {
			const char* reason = stbi_failure_reason();
			std::wstring err = L"Failed to load heightmap: " +
				std::wstring(reason, reason + strlen(reason));
			Logger::ERR(err);
			return false;
		}

		assert(static_cast<size_t>(width * height) == 1089 && "Image dimensions do not match the expected vertex grid size!");

		std::copy(rawData, rawData + 1089, outPixelArray.begin());
		stbi_image_free(rawData);

		return true;
	}
#pragma endregion
}