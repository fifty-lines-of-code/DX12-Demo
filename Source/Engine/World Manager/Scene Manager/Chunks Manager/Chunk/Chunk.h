#pragma once

#include "../../../../Math/EngineMath.h"


namespace Engine {

	class Chunk {
	public:
		Chunk();
		~Chunk();

		bool Initialize(Vector3& center);

	private:
		Vector3 mCenter;
	};
}