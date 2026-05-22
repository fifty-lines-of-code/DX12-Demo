#pragma once

#include "Geometry.h"
#include <vector>
#include <wrl.h>

enum class MeshID : uint32_t {
	Cube = 0,
	Count
};

class Mesh {
public:
	Mesh(MeshID id, std::vector<Vertex> vertices, std::vector<uint16_t> indices);
	~Mesh();

	const MeshID meshID;
	const UINT vbByteSize;
	const UINT ibByteSize;

	const std::vector<Vertex>& GetVertices() const;
	const std::vector<uint16_t>& GetIndices() const;

private:
	std::vector<Vertex> mVertices;
	std::vector<uint16_t> mIndices;
};