#pragma once

#include "DebugGameState.h"
#include <wtypes.h>

class GameState {
public:
	GameState(UINT windowedClientWidth, UINT windowedClientHeight);
	~GameState();

	bool GetIsPaused() const;
	bool GetIsMinimized() const;
	bool GetIsMaximized() const;
	bool GetIsResizing() const;
	bool GetIsFullscreen() const;
	bool GetIsDrawingDebugState() const;
	UINT GetWindowedClientWidth() const;
	UINT GetWindowedClientHeight() const;
	UINT GetFullscreenClientWidth() const;
	UINT GetFullscreenClientHeight() const;

	void SetIsPaused(bool paused);
	void SetIsMinimized(bool minimized);
	void SetIsMaximized(bool maximized);
	void SetIsResizing(bool resizing);
	void SetIsFullscreen(bool fullscreenEnabled);
	void SetWindowedClientWidth(UINT windowedClientWidth);
	void SetWindowedClientHeight(UINT windowedClientHeight);
	void SetFullscreenClientWidth(UINT fullscreenClientWidth);
	void SetFullscreenClientHeight(UINT fullscreenClientHeight);
	void SetIsDrawingDebugState(bool isDrawingDebugState);

private:
	UINT mWindowedClientWidth;
	UINT mWindowedClientHeight;
	UINT mFullscreenClientWidth;
	UINT mFullscreenClientHeight;
	bool mPaused = false;  // is the application paused?
	bool mMinimized = false;  // is the application minimized?
	bool mMaximized = false;  // is the application maximized?
	bool mResizing = false;   // are the resize bars being dragged?
	bool mIsFullscreen = false; // we start in windowed
	DebugGameState mDebugGameState;
};