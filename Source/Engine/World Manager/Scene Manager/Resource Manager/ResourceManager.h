#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include "../Entity/Mesh/Mesh.h"

namespace Engine {

	class ResourceManager {
	public:
		ResourceManager();
		~ResourceManager();

		const Mesh* GetMesh(MeshID id);

	private:
		std::vector<Mesh> mMeshes;
		// assuming we will have 64 unique Meshes
		// if we have more than that, time to create an array
		// and index into the uint64 inside the array based on div by 64 and modulo 64
		uint64_t mLoadedBitMask;

	private:
		static void CreateCubeMesh(Mesh* mesh);
		bool GetIsLoaded(size_t index);
		void SetIsLoaded(size_t index);
		void SetIsUnloaded(size_t index);
	};
}