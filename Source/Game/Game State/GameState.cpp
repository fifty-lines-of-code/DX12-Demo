#include "GameState.h"

GameState::GameState(UINT windowedClientWidth, UINT windowedClientHeight) :
	mWindowedClientWidth(windowedClientWidth),
	mWindowedClientHeight(windowedClientHeight),
	mFullscreenClientWidth(0),
	mFullscreenClientHeight(0)
{}

GameState::~GameState() {}

bool GameState::GetIsPaused() const { return mPaused; }

bool GameState::GetIsMinimized() const { return mMinimized; }

bool GameState::GetIsMaximized() const { return mMaximized; }

bool GameState::GetIsResizing() const { return mResizing; }

bool GameState::GetIsFullscreen() const { return mIsFullscreen; }

bool GameState::GetIsDrawingDebugState() const { 
	return mDebugGameState.IsDrawingDebugState;
}

UINT GameState::GetWindowedClientWidth() const { return mWindowedClientWidth; } 

UINT GameState::GetWindowedClientHeight() const { return mWindowedClientHeight; }

UINT GameState::GetFullscreenClientWidth() const { return mFullscreenClientWidth; }

UINT GameState::GetFullscreenClientHeight() const { return mFullscreenClientHeight; }

void GameState::SetIsPaused(bool paused) { mPaused = paused; }

void GameState::SetIsMinimized(bool minimized) { mMinimized = minimized; }

void GameState::SetIsMaximized(bool maximized) { mMaximized = maximized; }

void GameState::SetIsResizing(bool resizing) { mResizing = resizing; }

void GameState::SetIsFullscreen(bool fullscreenEnabled) { mIsFullscreen = fullscreenEnabled; }

void GameState::SetIsDrawingDebugState(bool isDrawingDebugState) { 
	mDebugGameState.IsDrawingDebugState = isDrawingDebugState;
}

void GameState::SetWindowedClientWidth(UINT windowedClientWidth) {
	mWindowedClientWidth = windowedClientWidth;
}

void GameState::SetWindowedClientHeight(UINT windowedlClientHeight) {
	mWindowedClientHeight = windowedlClientHeight;
}

void GameState::SetFullscreenClientWidth(UINT fullscreenClientWidth) {
	mFullscreenClientWidth = fullscreenClientWidth; 
}

void GameState::SetFullscreenClientHeight(UINT fullscreenClienHeight) {
	mFullscreenClientHeight = fullscreenClienHeight;
}

