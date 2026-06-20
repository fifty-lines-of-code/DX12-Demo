#include "DebugSystem.h"
#include <algorithm>

namespace Engine::DebugSystem {

	DebugSystem::DebugSystem() :
		mWindowWidth(0),
		mWindowHeight(0),
		mActiveLogCount(0),
		mTotalNumberOfCharactersDrawn(0),
		mIsDirty(false)
	{
		SetupFontAtlasForDebugFont();
		mUVCellWidth = mFontAtlasDesc.CellWidth / (float)mFontAtlasDesc.TextureWidth;
	}

	DebugSystem::~DebugSystem() {}

	void DebugSystem::Initialize(uint32_t windowWidth, uint32_t windowHeight) {
		mWindowWidth = windowWidth;
		mWindowHeight = windowHeight;
	}

	void DebugSystem::LogText(const std::string& text, const Vector4& color) {
		if (mActiveLogCount >= DebugLimits::MAX_CHARACTERS) { return; }

		mIsDirty = true;

		for (int i = 0; i < text.length(); ++i) {
			if (mActiveLogCount >= DebugLimits::MAX_CHARACTERS) { break; }

			DebugTextSlot& slot = mTextPool[mActiveLogCount];

			slot.Color = color;
			slot.Character = text[i];

			mActiveLogCount++;
		}
	}

	void DebugSystem::CalculateFramePositions() {
		if (!mIsDirty) { return; }

		uint32_t penX = DebugLimits::MARGIN;
		uint32_t penY = DebugLimits::MARGIN;
		uint32_t drawableWidth = mWindowWidth - (2 * DebugLimits::MARGIN);
		uint32_t drawableHeight = mWindowHeight - (2 * DebugLimits::MARGIN);

		mTotalNumberOfCharactersDrawn = 0;

		for (uint32_t i = 0; i < mActiveLogCount; ++i) {
			DebugTextSlot& slot = mTextPool[i];

			if (slot.Character == '\n') {
				penX = DebugLimits::MARGIN;
				penY += DebugLimits::QUAD_HEIGHT + DebugLimits::LINE_SPACING;
				if (penY + DebugLimits::QUAD_HEIGHT >= drawableHeight) { break; }

				continue;
			}

			if (penX + DebugLimits::QUAD_WIDTH >= drawableWidth) {
				penX = DebugLimits::MARGIN;
				penY += DebugLimits::QUAD_HEIGHT + DebugLimits::LINE_SPACING;
			}

			if (penY + DebugLimits::QUAD_HEIGHT >= drawableHeight) { break; }

			// in the case of \n we don't draw it so 
			// we index into the compiled vertices array using mTotalNumberOfCharactersDrawn
			DebugSystemPerCharacterData& vertex = mVertices[mTotalNumberOfCharactersDrawn];

			vertex.Position.x = penX;
			vertex.Position.y = penY;
			vertex.Color = slot.Color;

			// Safety fallback - lets display ? if someone sends in an incompatible char
			if (slot.Character < DebugLimits::FIRST_PRINTABLE_CHAR ||
				slot.Character > DebugLimits::LAST_PRINTABLE_CHAR) {
				slot.Character = '?';
			}

			// we send zero based indexing into the shader
			vertex.Ascii = slot.Character - DebugLimits::FIRST_PRINTABLE_CHAR;

			penX += DebugLimits::QUAD_WIDTH;
			mTotalNumberOfCharactersDrawn++;
		}
	}

	void DebugSystem::ClearFrameCache() {
		mActiveLogCount = 0;
		mTotalNumberOfCharactersDrawn = 0;
		mIsDirty = false;
	}

	void DebugSystem::UpdateWindowDimensions(uint32_t width, uint32_t height) {
		mWindowWidth = width;
		mWindowHeight = height;
	}

	bool DebugSystem::GetIsDirty() const noexcept { return mIsDirty; }

	void DebugSystem::SetIsDirty(bool isDirty) noexcept { 
		mIsDirty = isDirty;
	}

	const TextVerticesArray& DebugSystem::GetVertices() const noexcept {
		return mVertices;
	}

	uint32_t DebugSystem::GetTotalNumberOfCharacersToDraw() const noexcept { 
		return mTotalNumberOfCharactersDrawn; 
	}

	void DebugSystem::GetPerPassCbData(DebugSystemPerPassCbData& data) const noexcept {
		data.WindowWidth = mWindowWidth;
		data.WindowHeight = mWindowHeight;
		data.UV_CellWidth = mUVCellWidth;
	}

	void DebugSystem::SetupFontAtlasForDebugFont() {
		// 950 and 20 come from inspecting the dds file
		// todo: maybe write a parser of some sort
		mFontAtlasDesc.TextureWidth = 950;

		// again obtained from inspecting the dds
		mFontAtlasDesc.CellWidth = 10;
	}
}