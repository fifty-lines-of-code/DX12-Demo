#include "Mesh.h"

Mesh::Mesh() : 
	mMeshID(MeshID::Count),
	mVbByteSize(0),
	mIbByteSize(0)
{}

Mesh::~Mesh() {}

void Mesh::Load(MeshID id, std::vector<Engine::Vertex> vertices, std::vector<uint16_t> indices)
{
	mMeshID = id;
	mVertices = vertices;
	mIndices = indices;

	mVbByteSize = (UINT)vertices.size() * sizeof(Engine::Vertex);
	mIbByteSize = (UINT)indices.size() * sizeof(uint16_t);
}

const std::vector<Engine::Vertex>& Mesh::GetVertices() const {
	return mVertices;
}

const std::vector<uint16_t>& Mesh::GetIndices() const {
	return mIndices;
}

MeshID Mesh::GetMeshID() const { return mMeshID; }

UINT Mesh::GetVbByteSize() const { return mVbByteSize; }

UINT Mesh::GetIbByteSize() const { return mIbByteSize; }