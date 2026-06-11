#include "TextureManager.h"

#include <filesystem>
#include "../../../../../Helper/Logger.h"

namespace Engine::EngineResources {

	TextureManager::TextureManager() {}

	TextureManager::~TextureManager() {}

	bool TextureManager::Initialize() {
		// todo:
		LoadTextures();

		return true;
	}

	TextureArray& TextureManager::GetTextures() noexcept { return mTextureAssets; }

	bool TextureManager::LoadTextures() {
		// load the wood crate texture
		std::string name = "WoodCrate01.dds";
		std::wstring filename = L"Source\\Resources\\Textures\\WoodCrate01.dds";
		if (!RegisterTexture(name, filename, (uint32_t)TextureID::WOOD_CRATE)) { return false; }
		
		return true;
	}

	bool TextureManager::RegisterTexture(std::string& name, std::wstring& fileName, uint32_t id) {
		if (id < 0 || id >= (uint32_t)TextureID::COUNT) {
			return false;
		}

		if (!std::filesystem::exists(fileName)) {
			// Breakpoint here! Your path is wrong.
			Logger::ERR(L"ERROR: Texture file not found at path: " + fileName + L"\n");
			return false;
		}

		TextureAsset& tAsset = mTextureAssets[id];
		tAsset.Name = name;
		tAsset.FileName = fileName;
		tAsset.id = (TextureID)id;
		tAsset.isReadyToLoad = true;

		return true;
	}
}