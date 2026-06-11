#pragma once

#include <array>
#include "../../../../EngineConfig.h"
#include "Texture Asset/TextureAsset.h"

namespace Engine::EngineResources {

	using TextureArray = std::array<TextureAsset, EngineConfig::EngineConfig::MAX_TEXTURES>;

	class TextureManager {
	public:
		TextureManager();
		~TextureManager();

		bool Initialize();

		TextureArray& GetTextures() noexcept;

	private:
		TextureArray mTextureAssets;

	private:
		bool LoadTextures();
		bool RegisterTexture(std::string& name, std::wstring& fileName, uint32_t id);
	};
}