#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include "../Entities/Mesh/Mesh.h"

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	const Mesh* GetMesh(MeshID id);

private:
	std::array<std::unique_ptr<Mesh>, size_t(MeshID:: Count)> mMeshes;

private:
	Mesh* LoadMesh(MeshID id);
	static std::unique_ptr<Mesh> CreateCubeMesh();
};