#include "ResourceManager.h"

#include "../../Scene Manager/Entities/Mesh/Mesh.h"
#include <DirectXColors.h>

using namespace DirectX;

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

const Mesh* ResourceManager::GetMesh(MeshID id) {
	if (id >= MeshID::Count) {
		return nullptr;
	}

	if (mMeshes[size_t(id)] == nullptr) {
		switch (id) {
		case MeshID::Cube:
			return LoadMesh(id);
			break;

		default:
			return nullptr;
		}
	}

	return mMeshes[size_t(id)].get();
}

Mesh* ResourceManager::LoadMesh(MeshID id) {
	if (mMeshes[size_t(id)] == nullptr) {
		mMeshes[size_t(id)] = std::move(CreateCubeMesh());
	}
	
	return mMeshes[size_t(id)].get();
}

std::unique_ptr<Mesh> ResourceManager::CreateCubeMesh() {
	std::vector<Vertex> vertices = {
		Vertex({ XMFLOAT3(-.5f, -.5f, -.5f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-.5f, +.5f, -.5f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+.5f, +.5f, -.5f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+.5f, -.5f, -.5f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-.5f, -.5f, +.5f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-.5f, +.5f, +.5f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+.5f, +.5f, +.5f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+.5f, -.5f, +.5f), XMFLOAT4(Colors::Magenta) })
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
	return std::make_unique<Mesh>(MeshID::Cube, vertices, indices);
}
