#include "GameState.h"

GameState::GameState() {}

GameState::~GameState() {}

bool GameState::GetIsPaused() const { return mPaused; }

bool GameState::GetIsMinimized() const { return mMinimized; }

bool GameState::GetIsMaximized() const { return mMaximized; }

bool GameState::GetIsResizing() const { return mResizing; }

bool GameState::GetIsFullscreenEnabled() const { return mFullscreenState; }

void GameState::SetIsPaused(bool paused) { mPaused = paused; }

void GameState::SetIsMinimized(bool minimized) { mMinimized = minimized; }

void GameState::SetIsMaximized(bool maximized) { mMaximized = maximized; }

void GameState::SetIsResizing(bool resizing) { mResizing = resizing; }

void GameState::SetIsFullscreenEnabled(bool fullscreenEnabled) { mFullscreenState = fullscreenEnabled; }