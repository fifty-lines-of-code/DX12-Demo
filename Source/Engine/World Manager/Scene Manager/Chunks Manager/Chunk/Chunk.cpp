#include "Chunk.h"

namespace Engine {

	Chunk::Chunk() {}

	Chunk::~Chunk() {}

	bool Chunk::Initialize(Vector3& center) {
		mCenter = center;
		return true;
	}
}