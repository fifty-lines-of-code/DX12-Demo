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
	Mesh();
	~Mesh();

	void Load(MeshID id, std::vector<Engine::Vertex> vertices, std::vector<uint16_t> indices);

	const std::vector<Engine::Vertex>& GetVertices() const;
	const std::vector<uint16_t>& GetIndices() const;
	MeshID GetMeshID() const;
	UINT GetVbByteSize() const;
	UINT GetIbByteSize() const;
	const Engine::Vector3& GetLocalMin() const;
	const Engine::Vector3& GetLocalMax() const;

private:
	std::vector<Engine::Vertex> mVertices;
	std::vector<uint16_t> mIndices;

	MeshID mMeshID;
	UINT mVbByteSize;
	UINT mIbByteSize;
	Engine::Vector3 mLocalMin;
	Engine::Vector3 mLocalMax;

private:
	void CalculateLocalMinAndMax();
};