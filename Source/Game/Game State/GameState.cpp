#include "GameState.h"

GameState::GameState() {}

GameState::~GameState() {}

bool GameState::GetIsPaused() { return mPaused; }

bool GameState::GetIsMinimized() { return mMinimized; }

bool GameState::GetIsMaximized() { return mMaximized; }

bool GameState::GetIsResizing() { return mResizing; }

bool GameState::GetIsFullscreenEnabled() { return mFullscreenState; }

void GameState::SetIsPaused(bool paused) { mPaused = paused; }

void GameState::SetIsMinimized(bool minimized) { mMinimized = minimized; }

void GameState::SetIsMaximized(bool maximized) { mMaximized = maximized; }

void GameState::SetIsResizing(bool resizing) { mResizing = resizing; }

void GameState::SetIsFullscreenEnabled(bool fullscreenEnabled) { mFullscreenState = fullscreenEnabled; }