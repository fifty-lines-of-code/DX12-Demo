#include "TerrainManager.h"

#include <array>
#include "../../../../../Helper/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../../../Helper/stb_image.h"

namespace Engine {

	TerrainManager::TerrainManager(uint16_t chunkSize) : 
		mChunkSize(chunkSize),
		mChunkSizeHalf(chunkSize/2),
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
		std::vector<Vertex>& outVertices, 
		std::vector<uint16_t>& outIndices
	) {
		float startX = chunkCenterX - mChunkSizeHalf;
		float startZ = chunkCenterZ - mChunkSizeHalf;

		// Create the fixed-size array to hold the grayscale image data
		std::array<uint8_t, 1089> heightValues{};

		// Load the image straight into our array. 
		// (Because stbi_set_flip_vertically_on_load was true, the data layout is perfect!)
		if (!LoadHeightmapToArray("Source/Resources/Heightmaps/Chunk0x0.png", heightValues)) {
			// Fallback or assert if the file is missing
			assert(false && "Terrain texture asset could not be processed.");
		}

		uint16_t indexPtr = 0;
		Vertex v;

		for (int z = 0; z < mNumberOfVerticesPerEdge; ++z) {
			for (int x = 0; x < mNumberOfVerticesPerEdge; ++x) {
				uint16_t vIndex = (z * mNumberOfVerticesPerEdge) + x;

				v.Position.x = startX + static_cast<float>(x);
				v.Position.z = startZ + static_cast<float>(z);

				float normalizedHeight = static_cast<float>(heightValues[vIndex]) / 255.0f;
				v.Position.y = normalizedHeight * mMaxHeight;

				// Diagnostic Coloring: Matches the heightmap values visually on screen
				v.Color = Vector4(normalizedHeight, normalizedHeight, normalizedHeight, 1.0f);

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

#pragma region Private
	bool TerrainManager::LoadHeightmapToArray(
		const std::string& filename,
		std::array<uint8_t, 1089>& outPixelArray
	) {
		// 1. Tell stb_image to un-invert the rows vertically so it matches our 3D Z-axis
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channelsInFile = 0;

		// 2. Load and force stb_image to convert MS Paint's RGB down to 1 grayscale byte (last parameter: 1)
		uint8_t* rawData = stbi_load(filename.c_str(), &width, &height, &channelsInFile, 1);

		if (!rawData) {
			const char* reason = stbi_failure_reason();
			std::wstring err = L"Failed to load heightmap: " +
				std::wstring(reason, reason + strlen(reason));
			Logger::ERR(err);
			return false;
		}

		// 3. Safety check: Ensure the image pixels exactly match our compile-time array size
		assert(static_cast<size_t>(width * height) == 1089 && "Image dimensions do not match the expected vertex grid size!");

		// 4. Copy the raw memory bytes straight into our modern std::array container
		std::copy(rawData, rawData + 1089, outPixelArray.begin());

		// 5. Always clean up the raw C-allocated pointer memory
		stbi_image_free(rawData);

		return true;
	}
#pragma endregion
}