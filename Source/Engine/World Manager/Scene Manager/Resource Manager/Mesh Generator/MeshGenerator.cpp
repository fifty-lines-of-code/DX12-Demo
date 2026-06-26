#include "MeshGenerator.h"

namespace Engine::EngineResources {

    void MeshGenerator::GenerateCubeMesh(
        std::vector<Vertex>& outVertices,
        std::vector<uint16_t>& outIndices,
        std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH>& subMeshes
    ) const noexcept
    {
        outVertices.reserve(24);
        outIndices.reserve(36);

        // STANDARDIZED: -Z is Absolute Forward Convention
        Vector3 normals[6] = {
            Vector3(0.0f,  0.0f, -1.0f), // 0: Front Face (Z = -1)
            Vector3(1.0f,  0.0f,  0.0f), // 1: Right Face (X = +1)
            Vector3(-1.0f, 0.0f,  0.0f), // 2: Left Face  (X = -1)
            Vector3(0.0f,  1.0f,  0.0f), // 3: Top Face   (Y = +1)
            Vector3(0.0f, -1.0f,  0.0f), // 4: Bottom Face(Y = -1)
            Vector3(0.0f,  0.0f,  1.0f)  // 5: Back Face  (Z = +1)
        };

        // 0. Front Face (Z = -1). Viewed from -Z looking toward +Z, CW winding.
        outVertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[0], Vector2(0.0f, 0.0f) }); // 0: TL
        outVertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[0], Vector2(1.0f, 0.0f) }); // 1: TR
        outVertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[0], Vector2(1.0f, 1.0f) }); // 2: BR
        outVertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[0], Vector2(0.0f, 1.0f) }); // 3: BL

        // 1. Right Face (X = +1). Viewed from +X looking toward -X, CW winding.
        outVertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[1], Vector2(0.0f, 0.0f) }); // 4: TL
        outVertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[1], Vector2(1.0f, 0.0f) }); // 5: TR
        outVertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[1], Vector2(1.0f, 1.0f) }); // 6: BR
        outVertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[1], Vector2(0.0f, 1.0f) }); // 7: BL

        // 2. Left Face (X = -1). Viewed from -X looking toward +X, CW winding.
        outVertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[2], Vector2(0.0f, 0.0f) }); // 8: TL
        outVertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[2], Vector2(1.0f, 0.0f) }); // 9: TR
        outVertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[2], Vector2(1.0f, 1.0f) }); // 10: BR
        outVertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[2], Vector2(0.0f, 1.0f) }); // 11: BL

        // 3. Top Face (Y = +1). Viewed from +Y looking down, CW winding.
        outVertices.push_back({ Vector3(-1.0f, 1.0f,  1.0f), normals[3], Vector2(0.0f, 0.0f) }); // 12: TL
        outVertices.push_back({ Vector3(1.0f, 1.0f,  1.0f), normals[3], Vector2(1.0f, 0.0f) }); // 13: TR
        outVertices.push_back({ Vector3(1.0f, 1.0f, -1.0f), normals[3], Vector2(1.0f, 1.0f) }); // 14: BR
        outVertices.push_back({ Vector3(-1.0f, 1.0f, -1.0f), normals[3], Vector2(0.0f, 1.0f) }); // 15: BL

        // 4. Bottom Face (Y = -1). Viewed from -Y looking up, CW winding.
        outVertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[4], Vector2(0.0f, 0.0f) }); // 16: TL
        outVertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[4], Vector2(1.0f, 0.0f) }); // 17: TR
        outVertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[4], Vector2(1.0f, 1.0f) }); // 18: BR
        outVertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[4], Vector2(0.0f, 1.0f) }); // 19: BL

        // 5. Back Face (Z = +1). Viewed from +Z looking toward -Z, CW winding.
        outVertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[5], Vector2(0.0f, 0.0f) }); // 20: TL
        outVertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[5], Vector2(1.0f, 0.0f) }); // 21: TR
        outVertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[5], Vector2(1.0f, 1.0f) }); // 22: BR
        outVertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[5], Vector2(0.0f, 1.0f) }); // 23: BL

        // Index Winding Generation Loop (CW)
        for (uint16_t i = 0; i < 6; ++i) {
            uint16_t baseVertex = i * 4;
            outIndices.push_back(baseVertex + 0);
            outIndices.push_back(baseVertex + 1);
            outIndices.push_back(baseVertex + 2);
            outIndices.push_back(baseVertex + 0);
            outIndices.push_back(baseVertex + 2);
            outIndices.push_back(baseVertex + 3);
        }

        SubMesh& subMesh0 = subMeshes[0];
        subMesh0.IndexCount = static_cast<uint32_t>(outIndices.size());
        subMesh0.StartIndexLocation = 0;
        subMesh0.BaseVertexLocation = 0;
    }

    void MeshGenerator::GenerateCubeMesh_With_TwoSubMeshes(
        std::vector<Vertex>& outVertices,
        std::vector<uint16_t>& outIndices,
        std::array<SubMesh, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH>& subMeshes
    ) const noexcept
    {
        outVertices.reserve(24);
        outIndices.reserve(36);

        // STANDARDIZED: Matches cube normal array layout 1:1
        Vector3 normals[6] = {
            Vector3(0.0f,  0.0f, -1.0f), // 0: Glass surface facing -Z
            Vector3(1.0f,  0.0f,  0.0f), // 1: Right Face
            Vector3(-1.0f, 0.0f,  0.0f), // 2: Left Face
            Vector3(0.0f,  1.0f,  0.0f), // 3: Top Face
            Vector3(0.0f, -1.0f,  0.0f), // 4: Bottom Face
            Vector3(0.0f,  0.0f,  1.0f)  // 5: Back Face Opaque Backing
        };

        // =========================================================================
        // SUBMESH 0: The 5 Opaque Frame Backing Faces
        // =========================================================================

        // 1. Right Face (X = +1)
        outVertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[1], Vector2(0.0f, 0.0f) }); // 0: TL
        outVertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[1], Vector2(1.0f, 0.0f) }); // 1: TR
        outVertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[1], Vector2(1.0f, 1.0f) }); // 2: BR
        outVertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[1], Vector2(0.0f, 1.0f) }); // 3: BL

        // 2. Left Face (X = -1)
        outVertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[2], Vector2(0.0f, 0.0f) }); // 4: TL
        outVertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[2], Vector2(1.0f, 0.0f) }); // 5: TR
        outVertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[2], Vector2(1.0f, 1.0f) }); // 6: BR
        outVertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[2], Vector2(0.0f, 1.0f) }); // 7: BL

        // 3. Top Face (Y = +1)
        outVertices.push_back({ Vector3(-1.0f, 1.0f,  1.0f), normals[3], Vector2(0.0f, 0.0f) }); // 8: TL
        outVertices.push_back({ Vector3(1.0f, 1.0f,  1.0f), normals[3], Vector2(1.0f, 0.0f) }); // 9: TR
        outVertices.push_back({ Vector3(1.0f, 1.0f, -1.0f), normals[3], Vector2(1.0f, 1.0f) }); // 10: BR
        outVertices.push_back({ Vector3(-1.0f, 1.0f, -1.0f), normals[3], Vector2(0.0f, 1.0f) }); // 11: BL

        // 4. Bottom Face (Y = -1)
        outVertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[4], Vector2(0.0f, 0.0f) }); // 12: TL
        outVertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[4], Vector2(1.0f, 0.0f) }); // 13: TR
        outVertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[4], Vector2(1.0f, 1.0f) }); // 14: BR
        outVertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[4], Vector2(0.0f, 1.0f) }); // 15: BL

        // 5. Back Face (Z = +1)
        outVertices.push_back({ Vector3(1.0f,  1.0f,  1.0f), normals[5], Vector2(0.0f, 0.0f) }); // 16: TL
        outVertices.push_back({ Vector3(-1.0f,  1.0f,  1.0f), normals[5], Vector2(1.0f, 0.0f) }); // 17: TR
        outVertices.push_back({ Vector3(-1.0f, -1.0f,  1.0f), normals[5], Vector2(1.0f, 1.0f) }); // 18: BR
        outVertices.push_back({ Vector3(1.0f, -1.0f,  1.0f), normals[5], Vector2(0.0f, 1.0f) }); // 19: BL

        // Frame Indices (5 faces * 6 indices = 30 indices)
        for (uint16_t i = 0; i < 5; ++i) {
            uint16_t baseVertex = i * 4;
            outIndices.push_back(baseVertex + 0);
            outIndices.push_back(baseVertex + 1);
            outIndices.push_back(baseVertex + 2);
            outIndices.push_back(baseVertex + 0);
            outIndices.push_back(baseVertex + 2);
            outIndices.push_back(baseVertex + 3);
        }

        subMeshes[0].IndexCount = 30;
        subMeshes[0].StartIndexLocation = 0;
        subMeshes[0].BaseVertexLocation = 0;

        // =========================================================================
        // SUBMESH 1: Isolated Glass Surface (Z = -1, facing -Z)
        // =========================================================================

        // Glass face at Z=-1. Viewed from -Z looking toward +Z, CW winding.
        outVertices.push_back({ Vector3(-1.0f,  1.0f, -1.0f), normals[0], Vector2(0.0f, 0.0f) }); // 20: TL
        outVertices.push_back({ Vector3(1.0f,  1.0f, -1.0f), normals[0], Vector2(1.0f, 0.0f) }); // 21: TR
        outVertices.push_back({ Vector3(1.0f, -1.0f, -1.0f), normals[0], Vector2(1.0f, 1.0f) }); // 22: BR
        outVertices.push_back({ Vector3(-1.0f, -1.0f, -1.0f), normals[0], Vector2(0.0f, 1.0f) }); // 23: BL

        outIndices.push_back(20); outIndices.push_back(21); outIndices.push_back(22);
        outIndices.push_back(20); outIndices.push_back(22); outIndices.push_back(23);

        subMeshes[1].IndexCount = 6;
        subMeshes[1].StartIndexLocation = 30;
        subMeshes[1].BaseVertexLocation = 0;
    }
}