#pragma once
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
	UINT GetWindowedClientWidth() const;
	UINT GetWindowedClientHeight() const;

	void SetIsPaused(bool paused);
	void SetIsMinimized(bool minimized);
	void SetIsMaximized(bool maximized);
	void SetIsResizing(bool resizing);
	void SetIsFullscreen(bool fullscreenEnabled);
	void SetWindowedClientWidth(UINT fullscreenCLientWidth);
	void SetWindowedClientHeight(UINT fullscreenCLientHeight);
	void SetFullscreenClientWidth(UINT fullscreenCLientWidth);
	void SetFullscreenClientHeight(UINT fullscreenCLientHeight);

private:
	bool mPaused = false;  // is the application paused?
	bool mMinimized = false;  // is the application minimized?
	bool mMaximized = false;  // is the application maximized?
	bool mResizing = false;   // are the resize bars being dragged?
	bool mIsFullscreen = false; // fullscreen enabled

	UINT mWindowedClientWidth;
	UINT mWindowedClientHeight;
	UINT mFullscreenClientWidth;
	UINT mFullscreenClientHeight;
};