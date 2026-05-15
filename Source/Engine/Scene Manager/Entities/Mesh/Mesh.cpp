#include "Mesh.h"

Mesh::Mesh(MeshID id, std::vector<Vertex> vertices, std::vector<uint16_t> indices) :
	meshID(id),
	mVertices(vertices), 
	mIndices(indices),
	vbByteSize((UINT)vertices.size() * sizeof(Vertex)),
	ibByteSize((UINT)indices.size() * sizeof(uint16_t)) {
}

Mesh::~Mesh() {}

const std::vector<Vertex>& Mesh::GetVertices() const {
	return mVertices;
}

const std::vector<uint16_t>& Mesh::GetIndices() const {
	return mIndices;
}