#include "MeshGenerator.h"

namespace Engine::EngineResources {

	void MeshGenerator::GenerateCubeMesh(
		std::vector<Vertex>& outVertices,
		std::vector<uint16_t>& outIndices,
		std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH>& subMeshes
	) const noexcept 
	{
		// we have 4 vertices per face, thus 24 vertices
		outVertices.reserve(24);
		// and that evaluates to 36 indices
		outIndices.reserve(36);

		// Define the 6 unit directions for a cube's faces
		Vector3 normals[6] = {
			Vector3(0.0f,  0.0f, -1.0f), // Front (pointing out of screen toward eye)
			Vector3(0.0f,  0.0f,  1.0f), // Back  (pointing into screen away from eye)
			Vector3(1.0f,  0.0f,  0.0f), // Right
			Vector3(-1.0f,  0.0f,  0.0f), // Left
			Vector3(0.0f,  1.0f,  0.0f), // Top
			Vector3(0.0f, -1.0f,  0.0f)  // Bottom
		};

		// 1. Front Face (Z = -1, closest to camera. Looking straight at it, CW is TL -> TR -> BR -> BL)
		outVertices.push_back({
			Vector3(-1.0f,  1.0f, -1.0f),
			normals[0], 
			Vector2(0.0f, 0.0f) 
		}); // 0: Top-Left

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f, -1.0f),
			normals[0], 
			Vector2(1.0f, 0.0f) 
		}); // 1: Top-Right

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f, -1.0f),
			normals[0], 
			Vector2(1.0f, 1.0f)
		}); // 2: Bottom-Right

		outVertices.push_back({
			Vector3(-1.0f, -1.0f, -1.0f),
			normals[0], 
			Vector2(0.0f, 1.0f)
		}); // 3: Bottom-Left

		// 2. Back Face (Z = 1, furthest away. Looking from behind the cube, CW is TL -> TR -> BR -> BL)
		outVertices.push_back({ 
			Vector3(1.0f,  1.0f,  1.0f), 
			normals[1], 
			Vector2(0.0f, 0.0f) 
		}); // 4: Top-Left (from back view)

		outVertices.push_back({ 
			Vector3(-1.0f,  1.0f,  1.0f), 
			normals[1], 
			Vector2(1.0f, 0.0f)
		}); // 5: Top-Right (from back view)

		outVertices.push_back({
			Vector3(-1.0f, -1.0f,  1.0f), 
			normals[1], 
			Vector2(1.0f, 1.0f)
		}); // 6: Bottom-Right (from back view)

		outVertices.push_back({
			Vector3(1.0f, -1.0f,  1.0f), 
			normals[1],
			Vector2(0.0f, 1.0f)
		}); // 7: Bottom-Left (from back view)

		// 3. Right Face (X = 1. Looking straight at it, CW is TL -> TR -> BR -> BL)
		outVertices.push_back({
			Vector3(1.0f,  1.0f, -1.0f), 
			normals[2], 
			Vector2(0.0f, 0.0f)
		}); // 8: Top-Left

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f,  1.0f),
			normals[2],
			Vector2(1.0f, 0.0f) 
		}); // 9: Top-Right

		outVertices.push_back({
			Vector3(1.0f, -1.0f,  1.0f),
			normals[2], 
			Vector2(1.0f, 1.0f) 
		}); // 10: Bottom-Right

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f, -1.0f), 
			normals[2], 
			Vector2(0.0f, 1.0f) 
		}); // 11: Bottom-Left

		// 4. Left Face (X = -1. Looking straight at it, CW is TL -> TR -> BR -> BL)
		outVertices.push_back({ 
			Vector3(-1.0f,  1.0f,  1.0f), 
			normals[3], 
			Vector2(0.0f, 0.0f)
		}); // 12: Top-Left

		outVertices.push_back({
			Vector3(-1.0f,  1.0f, -1.0f),
			normals[3], 
			Vector2(1.0f, 0.0f)
		}); // 13: Top-Right

		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f, -1.0f), 
			normals[3], 
			Vector2(1.0f, 1.0f)
		}); // 14: Bottom-Right

		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f,  1.0f),
			normals[3], 
			Vector2(0.0f, 1.0f)
		}); // 15: Bottom-Left

		// 5. Top Face (Y = 1. Looking down at it from above, CW is TL -> TR -> BR -> BL)
		outVertices.push_back({
			Vector3(-1.0f,  1.0f,  1.0f), 
			normals[4], 
			Vector2(0.0f, 0.0f) 
		}); // 16: Top-Left


		outVertices.push_back({ 
			Vector3(1.0f,  1.0f,  1.0f), 
			normals[4],
			Vector2(1.0f, 0.0f) 
		}); // 17: Top-Right

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f, -1.0f), 
			normals[4],
			Vector2(1.0f, 1.0f) 
		}); // 18: Bottom-Right

		outVertices.push_back({ 
			Vector3(-1.0f,  1.0f, -1.0f), 
			normals[4], 
			Vector2(0.0f, 1.0f)
		}); // 19: Bottom-Left

		// 6. Bottom Face (Y = -1. Looking up at it from below, CW is TL -> TR -> BR -> BL)
		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f, -1.0f),
			normals[5],
			Vector2(0.0f, 0.0f)
		}); // 20: Top-Left

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f, -1.0f),
			normals[5], 
			Vector2(1.0f, 0.0f) 
		}); // 21: Top-Right

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f,  1.0f), 
			normals[5], 
			Vector2(1.0f, 1.0f) 
		}); // 22: Bottom-Right

		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f,  1.0f), 
			normals[5], 
			Vector2(0.0f, 1.0f) 
		}); // 23: Bottom-Left

		// Core Index Winding Loop
		for (uint16_t i = 0; i < 6; ++i) {
			uint16_t baseVertex = i * 4;

			// Triangle 1: Top-Left -> Top-Right -> Bottom-Right (Clockwise)
			outIndices.push_back(baseVertex + 0);
			outIndices.push_back(baseVertex + 1);
			outIndices.push_back(baseVertex + 2);

			// Triangle 2: Top-Left -> Bottom-Right -> Bottom-Left (Clockwise)
			outIndices.push_back(baseVertex + 0);
			outIndices.push_back(baseVertex + 2);
			outIndices.push_back(baseVertex + 3);
		}

		// Set the submeshes
		SubMesh& subMesh0 = subMeshes[0];

		subMesh0.IndexCount = (uint32_t)outIndices.size();
		subMesh0.StartIndexLocation = 0;
		subMesh0.BaseVertexLocation = 0;
	}

	void MeshGenerator::GenerateMirrorMesh(
		std::vector<Vertex>& outVertices,
		std::vector<uint16_t>& outIndices,
		std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH>& subMeshes
	) const noexcept 
	{
		outVertices.reserve(24);
		outIndices.reserve(36);

		// Define the 6 unit directions. 
		Vector3 normals[6] = {
			Vector3(0.0f,  0.0f,  1.0f), // 0: Back
			Vector3(1.0f,  0.0f,  0.0f), // 1: Right
			Vector3(-1.0f,  0.0f,  0.0f),// 2: Left
			Vector3(0.0f,  1.0f,  0.0f), // 3: Top
			Vector3(0.0f, -1.0f,  0.0f), // 4: Bottom
			Vector3(0.0f,  0.0f, -1.0f)  // 5: Front (The Mirror Surface)
		};

		// =========================================================================
		// SUBMESH 0: The 5 Body Faces (The Frame/Backing)
		// Vertices 0 through 19. Indices 0 through 29.
		// =========================================================================

		// 1. Back Face (Z = 1)
		outVertices.push_back({ 
			Vector3(1.0f,  1.0f,  1.0f),
			normals[0],
			Vector2(0.0f, 0.0f) 
		}); // 0

		outVertices.push_back({
			Vector3(-1.0f,  1.0f,  1.0f), 
			normals[0], 
			Vector2(1.0f, 0.0f)
		}); // 1

		outVertices.push_back({
			Vector3(-1.0f, -1.0f,  1.0f), 
			normals[0], 
			Vector2(1.0f, 1.0f)
		}); // 2

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f,  1.0f),
			normals[0], 
			Vector2(0.0f, 1.0f)
		}); // 3

		// 2. Right Face (X = 1)
		outVertices.push_back({ 
			Vector3(1.0f,  1.0f, -1.0f), 
			normals[1], 
			Vector2(0.0f, 0.0f) 
		}); // 4

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f,  1.0f), 
			normals[1], 
			Vector2(1.0f, 0.0f)
		}); // 5

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f,  1.0f), 
			normals[1],
			Vector2(1.0f, 1.0f) 
		}); // 6

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f, -1.0f), 
			normals[1], 
			Vector2(0.0f, 1.0f) 
		}); // 7

		// 3. Left Face (X = -1)
		outVertices.push_back({ 
			Vector3(-1.0f,  1.0f,  1.0f), 
			normals[2], 
			Vector2(0.0f, 0.0f)
		}); // 8

		outVertices.push_back({
			Vector3(-1.0f,  1.0f, -1.0f),
			normals[2], 
			Vector2(1.0f, 0.0f)
		}); // 9

		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f, -1.0f),
			normals[2], 
			Vector2(1.0f, 1.0f) 
		}); // 10

		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f,  1.0f),
			normals[2], 
			Vector2(0.0f, 1.0f)
		}); // 11

		// 4. Top Face (Y = 1)
		outVertices.push_back({ 
			Vector3(-1.0f,  1.0f,  1.0f),
			normals[3],
			Vector2(0.0f, 0.0f) 
		}); // 12

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f,  1.0f), 
			normals[3], 
			Vector2(1.0f, 0.0f) 
		}); // 13

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f, -1.0f), 
			normals[3],
			Vector2(1.0f, 1.0f) 
		}); // 14

		outVertices.push_back({
			Vector3(-1.0f,  1.0f, -1.0f), 
			normals[3], 
			Vector2(0.0f, 1.0f) 
		}); // 15

		// 5. Bottom Face (Y = -1)
		outVertices.push_back({ 
			Vector3(-1.0f, -1.0f, -1.0f), 
			normals[4], 
			Vector2(0.0f, 0.0f)
		}); // 16

		outVertices.push_back({ 
			Vector3(1.0f, -1.0f, -1.0f),
			normals[4],
			Vector2(1.0f, 0.0f)
		}); // 17

		outVertices.push_back({
			Vector3(1.0f, -1.0f,  1.0f), 
			normals[4], 
			Vector2(1.0f, 1.0f) 
		}); // 18

		outVertices.push_back({
			Vector3(-1.0f, -1.0f,  1.0f),
			normals[4], 
			Vector2(0.0f, 1.0f) 
		}); // 19

		// Generate indices for the 5 body faces (5 faces * 6 indices = 30 indices)
		// Winding: Clockwise (DX12 Default Front-Facing)
		for (uint16_t i = 0; i < 5; ++i) {
			uint16_t baseVertex = i * 4;

			// Triangle 1: TL -> TR -> BR (Clockwise)
			outIndices.push_back(baseVertex + 0);
			outIndices.push_back(baseVertex + 1);
			outIndices.push_back(baseVertex + 2);

			// Triangle 2: TL -> BR -> BL (Clockwise)
			outIndices.push_back(baseVertex + 0);
			outIndices.push_back(baseVertex + 2);
			outIndices.push_back(baseVertex + 3);
		}

		// Configure Submesh 0 (The Body)
		subMeshes[0].IndexCount = 30;
		subMeshes[0].StartIndexLocation = 0;
		subMeshes[0].BaseVertexLocation = 0; // Indices are absolute (0-19), so offset is 0

		// =========================================================================
		// SUBMESH 1: The Mirror Face (Front)
		// Vertices 20 through 23. Indices 30 through 35.
		// =========================================================================

		// 6. Front Face (Z = -1, closest to camera. This is the reflective surface)
		outVertices.push_back({
			Vector3(-1.0f,  1.0f, -1.0f),
			normals[5], 
			Vector2(0.0f, 0.0f)
		}); // 20: TL

		outVertices.push_back({ 
			Vector3(1.0f,  1.0f, -1.0f),
			normals[5], 
			Vector2(1.0f, 0.0f)
		}); // 21: TR

		outVertices.push_back({
			Vector3(1.0f, -1.0f, -1.0f), 
			normals[5],
			Vector2(1.0f, 1.0f)
		}); // 22: BR

		outVertices.push_back({
			Vector3(-1.0f, -1.0f, -1.0f),
			normals[5],
			Vector2(0.0f, 1.0f) 
		}); // 23: BL

		// Triangle 1: TL -> TR -> BR (Clockwise)
		outIndices.push_back(20);
		outIndices.push_back(21);
		outIndices.push_back(22);

		// Triangle 2: TL -> BR -> BL (Clockwise)
		outIndices.push_back(20);
		outIndices.push_back(22);
		outIndices.push_back(23);

		// Configure Submesh 1 (The Mirror Surface)
		subMeshes[1].IndexCount = 6;
		subMeshes[1].StartIndexLocation = 30; // Starts exactly after the 30 indices of Submesh 0
		subMeshes[1].BaseVertexLocation = 0;  // Indices are absolute (20-23), so offset is 0
	}
}