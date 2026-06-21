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
		mChunkSizeHalf(chunkSize / 2)
	{
		stbi_set_flip_vertically_on_load(true);
	}

	TerrainManager::~TerrainManager() {}

	uint16_t TerrainManager::TotalNumberOfVerticesForChunk(TerrainLOD lod) const noexcept {
		int density = (uint8_t) lod;

		int numberOfVertices = mChunkSize * density;

		return (numberOfVertices + 1) * (numberOfVertices + 1);
	}

	uint32_t TerrainManager::TotalNumberOfIndicesForChunk(TerrainLOD lod) const noexcept {
		int density = (uint8_t) lod;
		int numberOfVerticesPerEdge = mChunkSize * density + 1;
		int numberOfQuads = numberOfVerticesPerEdge - 1;

		// 2 * total number of quads = total no of triangles
		// 3 * total no of triangles = total no of indices
		return (numberOfQuads * numberOfQuads) * 2 * 3;
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

		// our bitmap is 33x33 thus 1089 values in the bitmap
		std::array<uint8_t, 1089> heightValues{};

		if (!LoadHeightmapToArray("Source/Resources/Heightmaps/Chunk0x0-4.png", heightValues)) {
			assert(false && "Terrain texture asset could not be processed.");
		}

		// 1. Determine our vertex subdivision factor based on LOD
		int densityMultiplier = (uint8_t) lod;

		// 2. Calculate vertex configurations based on LOD
		int vertsPerEdgeLOD = (mChunkSize * densityMultiplier) + 1;

		// Perfect world-space spacing to ensure the physical bounds stay locked at mChunkSize (32)
		float vertexSpacing = static_cast<float>(mChunkSize) / static_cast<float>(vertsPerEdgeLOD - 1);

		// How far we advance across our 33x33 source image per vertex index loop step
		float imageSampleStride = 1.0f / static_cast<float>(densityMultiplier);

		Vertex v;

		// Lambda helper to sample height safely at fractional image coordinates
		auto sampleHeightLambda = [&](float imgX, float imgZ) -> float {
			if (imgX < 0.0f) imgX = 0.0f;
			if (imgX > static_cast<float>(mChunkSize)) imgX = static_cast<float>(mChunkSize);
			if (imgZ < 0.0f) imgZ = 0.0f;
			if (imgZ > static_cast<float>(mChunkSize)) imgZ = static_cast<float>(mChunkSize);

			int x0 = static_cast<int>(std::floor(imgX));
			int x1 = x0 < mChunkSize ? x0 + 1 : x0;
			int z0 = static_cast<int>(std::floor(imgZ));
			int z1 = z0 < mChunkSize ? z0 + 1 : z0;

			float tx = imgX - static_cast<float>(x0);
			float tz = imgZ - static_cast<float>(z0);

			const int imgStride = mChunkSize + 1;
			float h00 = static_cast<float>(heightValues[(z0 * imgStride) + x0]);
			float h10 = static_cast<float>(heightValues[(z0 * imgStride) + x1]);
			float h01 = static_cast<float>(heightValues[(z1 * imgStride) + x0]);
			float h11 = static_cast<float>(heightValues[(z1 * imgStride) + x1]);

			float h0 = h00 + tx * (h10 - h00);
			float h1 = h01 + tx * (h11 - h01);
			float finalSampledHeight = h0 + tz * (h1 - h0);

			// todo: remove the div by 55 once we have a better bitmap
			// 55 because currently the "brightest" region has
			// greyscale value of 55

			float normalizedHeight = finalSampledHeight / 55.f;
			if (normalizedHeight > 1.f) { normalizedHeight = 1.f; }
			return normalizedHeight * mMaxHeight;
		};

		// 3. Loop over our dynamic vertex density limits
		for (int z = 0; z < vertsPerEdgeLOD; ++z) {
			for (int x = 0; x < vertsPerEdgeLOD; ++x) {

				// Set absolute positions stretched cleanly across the chunk size
				v.Position.x = startX + (static_cast<float>(x) * vertexSpacing);
				v.Position.z = startZ + (static_cast<float>(z) * vertexSpacing);

				// Map the current loop index back into fractional floating image coordinates
				float imgX = static_cast<float>(x) * imageSampleStride;
				float imgZ = static_cast<float>(z) * imageSampleStride;

				// Sample height for the current vertex
				v.Position.y = sampleHeightLambda(imgX, imgZ);

				// 4. VERTEX NORMAL CALCULATION (Sobel/Finite Difference style)
				// Sample 4 tiny offset steps around our current point to determine the local slope
				float offset = 0.1f;
				float hLeft = sampleHeightLambda(imgX - offset, imgZ);
				float hRight = sampleHeightLambda(imgX + offset, imgZ);
				float hDown = sampleHeightLambda(imgX, imgZ - offset);
				float hUp = sampleHeightLambda(imgX, imgZ + offset);
				float worldStep = offset * vertexSpacing;

				// Tangent (X-axis) and Bitangent (Z-axis) vectors are
				// calculated that define the plane of the triangle
				Engine::Vector3 tangent(2.0f * worldStep, hRight - hLeft, 0.0f);
				Engine::Vector3 bitangent(0.0f, hUp - hDown, 2.0f * worldStep);

				// they are then crossed to find the perpendicular 
				// upward-facing vector, which is then normalized
				// as it represents pure direction (up).
				Engine::Vector3 normal = bitangent.Cross(tangent);
				normal.Normalize();
				v.Normal = normal;

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