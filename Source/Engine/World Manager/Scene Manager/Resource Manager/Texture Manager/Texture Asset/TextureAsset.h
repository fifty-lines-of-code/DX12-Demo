#pragma once

#include <string>
#include "../TextureID.h"

namespace Engine::EngineResources {

	struct TextureAsset {
		std::wstring FileName;
		TextureID id = TextureID::INVALID;
		bool isLoaded = false;
		bool isReadyToLoad = false;
	};
}