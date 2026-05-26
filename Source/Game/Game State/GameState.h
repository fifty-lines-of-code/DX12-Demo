#pragma once

class GameState {
public:
	GameState();
	~GameState();

	bool GetIsPaused() const;
	bool GetIsMinimized() const;
	bool GetIsMaximized() const;
	bool GetIsResizing() const;
	bool GetIsFullscreen() const;

	void SetIsPaused(bool paused);
	void SetIsMinimized(bool minimized);
	void SetIsMaximized(bool maximized);
	void SetIsResizing(bool resizing);
	void SetIsFullscreen(bool fullscreenEnabled);

private:

	bool mPaused = false;  // is the application paused?
	bool mMinimized = false;  // is the application minimized?
	bool mMaximized = false;  // is the application maximized?
	bool mResizing = false;   // are the resize bars being dragged?
	bool mIsFullscreen = false; // fullscreen enabled
};