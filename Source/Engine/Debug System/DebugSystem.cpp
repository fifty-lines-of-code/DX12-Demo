#include "DebugSystem.h"

namespace Engine::DebugSystem {

	DebugSystem::DebugSystem() :
		mWindowWidth(0),
		mWindowHeight(0),
		mActiveLogCount(0),
		mActiveVertexCount(0),
		mActiveIndexCount(0)
	{}

	DebugSystem::~DebugSystem() {}

	void DebugSystem::Initialize(const FontAtlasDesc& fontDesc, uint32_t windowWidth, uint32_t windowHeight) {
		mFontDesc = fontDesc;
		mWindowWidth = windowWidth;
		mWindowHeight = windowHeight;
	}

	void DebugSystem::LogText(const std::string& text, const Vector4& color) {
		// todo
		if (mActiveLogCount >= DebugLimits::MaxLogInstances) { return; }

		mTextInstancesPool[mActiveLogCount].Text = text; 
		mTextInstancesPool[mActiveLogCount].Color = color;
		mActiveLogCount++;
	}

	void DebugSystem::CompileFrameGeometry() {
		// todo
	}

	void DebugSystem::ClearFrameCache() {
		// todo
	}

	const TextVeticesArray& DebugSystem::GetVertices() const noexcept {
		return mCompiledVertices;
	}

	const std::array<uint16_t, DebugLimits::MaxIndices>& DebugSystem::GetIndices() const noexcept { 
		return mCompiledIndices; 
	}

	uint32_t DebugSystem::GetActiveVertexCount() const noexcept { return mActiveVertexCount; }

	uint32_t DebugSystem::GetActiveIndexCount()  const noexcept { return mActiveIndexCount; }
}