#include "ResourceManager.h"

#include "../../Scene Manager/Entities/Mesh/Mesh.h"
#include <DirectXColors.h>

using namespace DirectX;

ResourceManager::ResourceManager() : mLoadedBitMask(0) {
	mMeshes.resize(static_cast<size_t>(MeshID::Count));
}

ResourceManager::~ResourceManager() {
}

const Mesh* ResourceManager::GetMesh(MeshID id) {
	if (id >= MeshID::Count) {
		return nullptr;
	}

	size_t index = size_t(id);
	Mesh* mesh = nullptr;

	if (!GetIsLoaded(index)) {
		switch (id) {
		case MeshID::Cube:
			mesh = &mMeshes[index];
			CreateCubeMesh(mesh);
			SetIsLoaded(index);
			break;

		default:
			return nullptr;
		}
	}
	else {
		mesh = &mMeshes[index];
	}

	return mesh;
}

void ResourceManager::CreateCubeMesh(Mesh* mesh) {
	std::vector<Engine::Vertex> vertices = {
		Engine::Vertex({ Engine::Vector3(-.5f, -.5f, -.5f), Engine::Vector4(Colors::White.f) }),
		Engine::Vertex({ Engine::Vector3(-.5f, +.5f, -.5f), Engine::Vector4(Colors::Black.f) }),
		Engine::Vertex({ Engine::Vector3(+.5f, +.5f, -.5f), Engine::Vector4(Colors::Red.f) }),
		Engine::Vertex({ Engine::Vector3(+.5f, -.5f, -.5f), Engine::Vector4(Colors::Green.f) }),
		Engine::Vertex({ Engine::Vector3(-.5f, -.5f, +.5f), Engine::Vector4(Colors::Blue.f) }),
		Engine::Vertex({ Engine::Vector3(-.5f, +.5f, +.5f), Engine::Vector4(Colors::Yellow.f) }),
		Engine::Vertex({ Engine::Vector3(+.5f, +.5f, +.5f), Engine::Vector4(Colors::Cyan.f) }),
		Engine::Vertex({ Engine::Vector3(+.5f, -.5f, +.5f), Engine::Vector4(Colors::Magenta.f) })
	};

	std::vector<uint16_t> indices = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};
	mesh->Load(MeshID::Cube, vertices, indices);
}

bool ResourceManager::GetIsLoaded(size_t index) {
	assert(index >= 0 && index < 64);

	uint64_t mask = 1ULL << index;
	return (mLoadedBitMask & mask) != 0;
}

void ResourceManager::SetIsLoaded(size_t index) {
	assert(index >= 0 && index < 64);

	uint64_t mask = 1ULL << index;
	mLoadedBitMask |= mask;
}

void ResourceManager::SetIsUnloaded(size_t index) {
	assert(index >= 0 && index < 64);

	uint64_t mask = 1ULL << index;
	uint64_t maskNegate = ~mask;
	mLoadedBitMask &= maskNegate;
}
